#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>

#include "hex.h"
#include "led.h"

#define BASE_ADDR 0xFF200000
/*Register*/
#define BUTTON_OFFSET 0x50
#define LED_OFFSET 0x00
#define HEX_OFFSET_0_3 0x20
#define HEX_OFFSET_4_5 0x30

/* Masks */
#define BUT0_MASK 0x00000001
#define BUT1_MASK 0x00000002

/*values*/
#define MIN 0
#define MAX 99
#define DELAY_US 10000 // 10 ms

/// @brief Method to clear the hex display and the leds
void clear();

bool running = true; // Variable qui contrôle la boucle while

// Important: thanks to this function, the program will exit properly and the mem fd will be closed
void handle_sigint(int sig)
{
	printf("\nInterrupt signal caught\nProgram will now exit properly\n");
	running = false;
}

int main()
{
	/*Map the signal SIGINT to the function handle_sigint*/
	signal(SIGINT, handle_sigint);

	/*open memory file descriptor*/
	int mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (mem_fd == -1) {
		perror("Error: cannot open /dev/mem");
		return EXIT_FAILURE;
	}

	/*get page size of system*/
	const long page_size = sysconf(_SC_PAGESIZE);
	if (page_size == -1) {
		perror("Error: cannot get page size");
		return EXIT_FAILURE;
	}

	/*Map /dev/mem to an address at a multiple of the page size on BASE_ADDR*/
	uint8_t *base_addr = (uint8_t *)mmap(NULL, page_size,
					     PROT_READ | PROT_WRITE, MAP_SHARED,
					     mem_fd,
					     BASE_ADDR & ~(page_size - 1));
	if (base_addr == MAP_FAILED) {
		perror("Error: memory mapping failed");
		close(mem_fd);
		return -1;
	}

	/*Get pointers to the registers*/
	volatile uint32_t *btn_reg =
		(volatile uint32_t *)(base_addr + BUTTON_OFFSET);
	volatile uint32_t *hex_reg_0_3 =
		(volatile uint32_t *)(base_addr + HEX_OFFSET_0_3);
	volatile uint32_t *hex_reg_4_5 =
		(volatile uint32_t *)(base_addr + HEX_OFFSET_4_5);
	volatile uint32_t *led_reg =
		(volatile uint32_t *)(base_addr + LED_OFFSET);

	init_led(led_reg);
	init_hex(hex_reg_0_3, hex_reg_4_5);

	uint8_t counter = MIN;

	bool isPressed = false;
	uint8_t tens = 0;
	uint8_t unit = 0;

	while (running) {
        
        bool btn0 = *btn_reg & BUT0_MASK;
        bool btn1 = *btn_reg & BUT1_MASK;

		if (btn0 && !isPressed && counter < MAX) {
			counter++;
			isPressed = true;
            printf("counter: %d\n", counter);
		} else if (btn1 && !isPressed &&
			   counter > MIN) {
			counter--;
			isPressed = true;
            printf("counter: %d\n", counter);
		}

		if (*btn_reg == 0) {
			isPressed = false;
		}
		led_down(unit);
		unit = get_decimal_digit(counter, 0);
        display_decimal_number(counter);
		led_up(unit);

		usleep(DELAY_US);
	}

	munmap(base_addr, page_size);

	close(mem_fd);

	return EXIT_SUCCESS;
	// // Set hex register to 0
	// clear();

	// // Counter
	// int8_t counter = MIN;

	// while (1) {
	// 	bool isPressed =
	// 		false; // Boolean used for avoidng unnecessary access memory
	// 	const uint32_t button_value =
	// 		*btn_reg; // Current push button state

	// 	if (button_value & BUT0_MASK) {
	// 		if (counter < MAX) {
	// 			counter++;
	// 			isPressed = true;
	// 		}
	// 	} else if (button_value & BUT1_MASK) {
	// 		if (counter > MIN) {
	// 			counter--;
	// 			isPressed = true;
	// 		}
	// 	} else {
	// 		// Do nothing
	// 	}

	// 	if (isPressed) {
	// 		// Split tens and units
	// 		const uint8_t tens = counter / 10;
	// 		const uint8_t unit = counter % 10;

	// 		// Update value
	// 		*hex_reg = (DEC_TO_HEX[tens] << 8) |
	// 			       DEC_TO_HEX[unit];
	// 		*led_reg = (1 << unit);
	// 	}

	// 	usleep(DELAY_US);
	// }

	// clear(hex_display, leds);

	return 0;
}

void clear()
{
	clear_leds();
	led_up(0);
	clear_all_hex();
}
