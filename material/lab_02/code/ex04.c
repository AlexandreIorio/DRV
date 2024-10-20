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
#define SWITCH_MASK 0x3FF
#define SWITCH_SIGN_MASK 0x200
#define SWITCH_VALUE_MASK ~SWITCH_SIGN_MASK &SWITCH_MASK

/* Constants */

#define SIGN_LED 9
#define MAX_VALUE 999999

static struct {
	volatile uint32_t *base_addr;
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

/// @brief Method to add the value of the switches
/// @param value The value to add
void add(int value);

/// @brief Method to substract the value of the switches
/// @param value The value to substract
void sub(int value);

/// @brief Method to multiply the value of the switches
/// @param value The value to multiply
void mult(int value);

/// @brief Method to divide the value of the switches
/// @param value The value to divide
void divi(int value);

/// @brief Method to finish the system
void finish();

bool running = true; // Variable qui contrôle la boucle while

// Important: this function ensures the program exits properly and closes the mem fd
void handle_sigint(int sig)
{
	printf("Interrupt signal caught\nProgram will now exit properly when the thread will be released\n");
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
	printf("Press KEY0 to add the value of the switches to the accumulator\n");
	printf("Press KEY1 to substract the value of the switches to the accumulator\n");
	printf("Press KEY2 to multiply the value of the switches to the accumulator\n");
	printf("Press KEY3 to divide the value of the switches to the accumulator\n");
	printf("Press KEY0 and KEY3 or CTRL+C to exit the program properly \n");
	printf("\n---------------------------------\n\n");

	while (running) {
		uint32_t info = 1; /*unmask*/

		size_t nb = write(counter_ctl.uio_fd, &info, sizeof(info));
		if (nb != (ssize_t)sizeof(info)) {
			logMessage(ERROR, "Cannot write to UIO device\n");
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

	init_button(btn_reg);
	init_switch(switch_reg);
	init_led(led_reg);
	init_hex(hex_reg_0_3, hex_reg_4_5);

	button_enable_interrupts(BUTTON_MASK);

	// initialize the display to 0
	display_digit(0, 0);

	logMessage(INFO, "Initialization completed\n");
	return 0;
}

void process()
{
	uint8_t pressed_btn_mask = button_status_interrupts(0xf);
	logMessage(DEBUG, "Pressed button mask: %d\n", pressed_btn_mask);
	uint16_t switches = read_all_switches();
	int value;

	// define the value of the switches
	if (switches & SWITCH_SIGN_MASK) {
		value = ~(switches & SWITCH_VALUE_MASK) + 1;
	} else {
		value = switches & SWITCH_VALUE_MASK;
	}

	switch (pressed_btn_mask) {
	case 1:
		add(value);
		break;
	case 2:
		sub(value);
		break;
	case 4:
		mult(value);
		break;

	case 8:
		divi(value);
		break;

	case 9:
		running = false;
		return;

	default:
		logMessage(DEBUG, " pressed button not in switch case\n");
		break;
	}

	// Check if the value is within the limits
	if (abs(counter_ctl.value) > MAX_VALUE) {
		logMessage(DEBUG, "Limit reached\n");
		if (counter_ctl.value > 0) {
			counter_ctl.value = MAX_VALUE;
		} else {
			counter_ctl.value = ~MAX_VALUE + 1;
		}
	}

	// Update the sign LED
	if (counter_ctl.value < 0) {
		led_up(SIGN_LED);
	} else {
		led_down(SIGN_LED);
	}
	logMessage(INFO, "Value: %d\n", counter_ctl.value);
	display_value_on_displays(counter_ctl.value, 10);
}

void add(int value)
{
	logMessage(INFO, "Adding %d to the accumulator\n", value);
	counter_ctl.value += value;
}

void sub(int value)
{
	logMessage(INFO, "Substracting %d to the accumulator\n", value);
	counter_ctl.value -= value;
}

void mult(int value)
{
	logMessage(INFO, "Multiplying the accumulator by %d\n", value);
	counter_ctl.value *= value;
}

void divi(int value)
{
	logMessage(INFO, "Dividing the accumulator by %d\n", value);
	if (value == 0) {
		logMessage(WARNING,
			   "Error: division by zero can be dangerous\n");
		return;
	}
	counter_ctl.value /= value;
}

void finish()
{
	clear();
	logMessage(DEBUG, "Unmapping memory\n");
	munmap((uint32_t *)counter_ctl.base_addr, 0);
	logMessage(DEBUG, "Closing memory file descriptor\n");
	close(counter_ctl.uio_fd);
	logMessage(INFO, "Program finished\n");
}
