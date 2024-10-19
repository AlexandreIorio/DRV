#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "button.h"
#include "hex.h"
#include "led.h"
#include "switch.h"
#include "logger.h"

#define UIO_DEVICE "/dev/uio0"
#define MAP_SIZE 0x1000 // Taille définie dans le device tree

/* Register offsets */
#define BUTTON_OFFSET 0x50
#define SWITCH_OFFSET 0x40
#define LED_OFFSET 0x00
#define HEX_OFFSET_0_3 0x20
#define HEX_OFFSET_4_5 0x30
#define IRQ_ENABLE_OFFSET 0x8
#define IRQ_STATUS_OFFSET 0xC

/*Masks*/
#define BUTTON_MASK 0xF

#define LONG_PRESS_DURATION 1500
#define DELAY_US 100000

#define MAX_VALUE 999999

static struct {
	volatile uint32_t *base_addr;
	;
	int page_size;
	int uio_fd;
	int value;

} counter_ctl;

/// @brief Method to clear the hex display and the leds
void clear();

/// @brief Method to initialize the system
/// @return 0 if the initialization is successful, -1 otherwise
int initialize();

/// @brief Method to process the system
void process();

/// @brief Method to finish the system
void finish();

bool running = true; // Variable qui contrôle la boucle while

// Important: this function ensures the program exits properly and closes the mem fd
void handle_sigint(int sig)
{
	logMessage(INFO,
		   "Interrupt signal caught\nProgram will now exit properly\n");
	running = false;
}

int main(int argc, char *argv[])
{
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
	printf("Press KEY0 to add the value of the switches and KEY1 to reset the accumulator\n");
	printf("Press and hold KEY0 to add value of the switches faster\n");
	printf("Press CTRL+C to exit properly\n\n");
	printf("---------------------------------\n\n");


	while (running) {
		uint32_t info = 1;

		size_t nb = write(counter_ctl.uio_fd, &info, sizeof(info));
		if (nb != (ssize_t)sizeof(info)) {
			perror("write");
			close(counter_ctl.uio_fd);
			exit(EXIT_FAILURE);
		}

		/* Wait for interrupt */
		logMessage(DEBUG, "Waiting for interrupt...\n");
		nb = read(counter_ctl.uio_fd, &info, sizeof(info));
		if (nb == (ssize_t)sizeof(info)) {
			/* Do something in response to the interrupt. */
			logMessage(DEBUG, "Interrupt #%u!\n", info);
            process();
			button_clear_interrupts(BUTTON_MASK);
		}
	}

	finish();
	return EXIT_SUCCESS;
}

void clear()
{
	clear_leds();
	clear_all_hex();
}

int initialize()
{
	logMessage(INFO, "Initialization started\n");

    // Initialize the control structure
    counter_ctl.value = 0;

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
		(uint32_t *)mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE,
				 MAP_SHARED, counter_ctl.uio_fd, 0);
	if (counter_ctl.base_addr == MAP_FAILED) {
		logMessage(ERROR, "Error: memory mapping failed");
		close(counter_ctl.uio_fd);
		return -1;
	}
	logMessage(DEBUG, "Memory mapped to base address %p\n",
		   counter_ctl.base_addr);

	volatile uint32_t *btn_reg =
		(volatile uint32_t *)((uint32_t)counter_ctl.base_addr |
				      BUTTON_OFFSET);

	volatile uint32_t *hex_reg_0_3 =
		(volatile uint32_t *)((uint32_t)counter_ctl.base_addr |
				      HEX_OFFSET_0_3);
	volatile uint32_t *hex_reg_4_5 =

		(volatile uint32_t *)((uint32_t)counter_ctl.base_addr |
				      HEX_OFFSET_4_5);
	volatile uint32_t *led_reg =
		(volatile uint32_t *)((uint32_t)counter_ctl.base_addr |
				      LED_OFFSET);
	volatile uint32_t *switch_reg =
		(volatile uint32_t *)((uint32_t)counter_ctl.base_addr |
				      SWITCH_OFFSET);

	logMessage(DEBUG, "led_reg: %p\n", led_reg);
	logMessage(DEBUG, "hex_reg_0_3: %p\n", hex_reg_0_3);
	logMessage(DEBUG, "hex_reg_4_5: %p\n", hex_reg_4_5);
	logMessage(DEBUG, "btn_reg: %p\n", btn_reg);
	logMessage(DEBUG, "switch_reg: %p\n", switch_reg);

	init_button(btn_reg);
	button_enable_interrupts(BUTTON_MASK);

	init_switch(switch_reg);
	init_led(led_reg);
	init_hex(hex_reg_0_3, hex_reg_4_5);

	logMessage(INFO, "Initialization completed\n");
	return 0;
}

void process()
{
    uint8_t pressed_btn_mask = button_status_interrupts(0x3);
    logMessage(DEBUG, "Pressed button mask: %d\n", pressed_btn_mask);
	int btn0 = pressed_btn_mask & 0x1;
	int btn1 = pressed_btn_mask & 0x2;

	if (btn0 == -1 || btn1 == -1) {
		logMessage(ERROR, "Error: cannot read button\n");
	}

	if (btn0) {
		if (counter_ctl.value < MAX_VALUE) {
			counter_ctl.value  += read_all_switches();
			if (counter_ctl.value > MAX_VALUE) {
				counter_ctl.value = MAX_VALUE;
			}
			display_value_on_displays(counter_ctl.value, 10);
		}
	} else if (btn1) {
		counter_ctl.value = 0;
		display_value_on_displays(counter_ctl.value, 10);
	}

	if (counter_ctl.value == MAX_VALUE) {
		led_up(0);
	}
}

void finish()
{
	clear();
	logMessage(DEBUG, "Unmapping memory\n");
	munmap((uint32_t *)counter_ctl.base_addr, MAP_SIZE);
	logMessage(DEBUG, "Closing memory file descriptor\n");
	close(counter_ctl.uio_fd);
	logMessage(INFO, "Program finished\n");
}
