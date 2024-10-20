#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "button.h"
#include "hex.h"
#include "led.h"
#include "logger.h"
#include "switch.h"

#define UIO_DEVICE "/dev/uio0"
#define MAP_SIZE 0x1000 // Taille définie dans le device tree

/* Register offsets */
#define BUTTON_OFFSET 0x50
#define SWITCH_OFFSET 0x40
#define LED_OFFSET 0x00
#define HEX_OFFSET_0_3 0x20
#define HEX_OFFSET_4_5 0x30

#define LONG_PRESS_DURATION 1500
#define DELAY_US 100000

#define MAX_VALUE 999999

static struct {
  volatile uint32_t *base_addr;
  int page_size;
  int uio_fd;
} counter_ctl;

/// @brief Method to clear the hex display and the leds
void clear();

/// @brief Method to initialize the system
/// @return 0 if the initialization is successful, -1 otherwise
int initialize();

/// @brief Method to finish the system
void finish();

bool running = true; // Variable qui contrôle la boucle while

// Important: this function ensures the program exits properly and closes the
// mem fd
void handle_sigint(int sig) {
  logMessage(INFO, "Interrupt signal caught\nProgram will now exit properly\n");
  running = false;
}

int main(int argc, char *argv[]) {
  if (argc == 3 && strcmp(argv[1], "--verbosity") == 0) {
    if (strcmp(argv[2], "DEBUG") == 0) {
      setLogLevel(DEBUG);
    } else if (strcmp(argv[2], "INFO") == 0) {
      setLogLevel(INFO);
    } else if (strcmp(argv[2], "WARNING") == 0) {
      setLogLevel(WARNING);
    } else if (strcmp(argv[2], "ERROR") == 0) {
      setLogLevel(ERROR);
    } else {
      logMessage(ERROR, "Invalid log level\n");
      return EXIT_FAILURE;
    }
    enableConsoleLogging(true);
  }

  /* Map the signal SIGINT to the function handle_sigint */
  signal(SIGINT, handle_sigint);

  logMessage(DEBUG, "Program started\n");

  if (initialize() == -1) {
    logMessage(INFO, "Error: initialization failed\n");
    return EXIT_FAILURE;
  }
  system("clear");

  printf("---------------------------------\n");
  printf("Welcome to the switch accumulator program\n");
  printf("---------------------------------\n\n");
  printf("Press KEY0 to add the value of the switches and KEY1 to reset the "
         "accumulator\n");
  printf("Press and hold KEY0 to add value of the switches faster\n");
  printf("Press CTRL+C to exit properly\n\n");
  printf("---------------------------------\n\n");

  bool isPressed = false;
  int value = 0;

  while (running) {
    int btn0 = read_button(0);
    int btn1 = read_button(1);

    if (btn0 == -1 || btn1 == -1) {
      logMessage(ERROR, "Error: cannot read button\n");
    }

    if ((btn0 && !isPressed) || (long_press(0, LONG_PRESS_DURATION) == 0)) {
      if (value < MAX_VALUE) {
        value += read_all_switches();
        if (value > MAX_VALUE) {
          value = MAX_VALUE;
        }
        isPressed = true;
        display_value_on_displays(value, 10);
      }
    } else if (btn1 && !isPressed) {
      value = 0;
      isPressed = true;
      display_value_on_displays(value, 10);
    }

    if (!(btn0 || btn1)) {
      isPressed = false;
    }

    if (value == MAX_VALUE) {
      led_up(0);
    }

    // Compute the delay if a button is long pressed
    long delay = (long_press(0, LONG_PRESS_DURATION) == 0 ||
                  long_press(1, LONG_PRESS_DURATION) == 0)
                     ? LONG_PRESS_DURATION
                     : DELAY_US;
    usleep(delay);
  }

  finish();
  return EXIT_SUCCESS;
}

void clear() {
  clear_leds();
  clear_all_hex();
}

int initialize() {
  logMessage(INFO, "Initialization started\n");

  counter_ctl.uio_fd = open(UIO_DEVICE, O_RDWR);
  logMessage(DEBUG, "Opening UIO device file descriptor\n");
  if (counter_ctl.uio_fd == -1) {
    logMessage(ERROR, "Error: cannot open UIO device");
    return -1;
  }

  logMessage(DEBUG, "Memory file descriptor opened\n");

  counter_ctl.page_size = sysconf(_SC_PAGESIZE);
  if (counter_ctl.page_size == -1) {
    logMessage(ERROR, "Error: cannot get page size");
    return -1;
  }

  logMessage(DEBUG, "Mapping memory\n");
  counter_ctl.base_addr =
      (uint32_t *)mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
                       counter_ctl.uio_fd, 0);
  if (counter_ctl.base_addr == MAP_FAILED) {
    logMessage(ERROR, "Error: memory mapping failed");
    close(counter_ctl.uio_fd);
    return -1;
  }
  logMessage(DEBUG, "Memory mapped\n");

  volatile uint32_t *btn_reg =
      (volatile uint32_t *)((void *)counter_ctl.base_addr + BUTTON_OFFSET);
  volatile uint32_t *hex_reg_0_3 =
      (volatile uint32_t *)((void *)counter_ctl.base_addr + HEX_OFFSET_0_3);
  volatile uint32_t *hex_reg_4_5 =
      (volatile uint32_t *)((void *)counter_ctl.base_addr + HEX_OFFSET_4_5);
  volatile uint32_t *led_reg =
      (volatile uint32_t *)((void *)counter_ctl.base_addr + LED_OFFSET);
  volatile uint32_t *switch_reg =
      (volatile uint32_t *)((void *)counter_ctl.base_addr + SWITCH_OFFSET);

  init_led(led_reg);
  init_hex(hex_reg_0_3, hex_reg_4_5);
  init_button(btn_reg);
  init_switch(switch_reg);
  logMessage(INFO, "Initialization completed\n");
  return 0;
}

void finish() {
  clear();
  logMessage(DEBUG, "Unmapping memory\n");
  munmap((uint32_t *)counter_ctl.base_addr, MAP_SIZE);
  logMessage(DEBUG, "Closing memory file descriptor\n");
  close(counter_ctl.uio_fd);
  logMessage(INFO, "Program finished\n");
}
