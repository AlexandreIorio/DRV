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

#include "hex.h"
#include "led.h"

#define BASE_ADDR 0xFF200000
/*Register*/
#define BUTTON_OFFSET 0x50
#define LED_OFFSET 0x00

/* Masks */
#define BUT0_MASK 0x00000001
#define BUT1_MASK 0x00000002

#define DELAY_US 10000 // 10 ms

void clear(volatile uint32_t *hex_display, volatile uint32_t *leds)
{
    clear_leds();
    led_up(0);
    clear_all_hex(hex_display, leds);
	int hex_0_value = (DEC_TO_HEX[MIN]);
	int hex_1_value = (DEC_TO_HEX[MIN]);
	int hex_initiale_value = (DEC_TO_HEX[MIN] << 8) | DEC_TO_HEX[MIN];
	*hex_display = hex_initiale_value;
	*leds = 0x00000001;
}





int main()
{
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
	volatile uint32_t *hex_reg =
		(volatile uint32_t *)(base_addr + HEX_OFFSET);
	volatile uint32_t *led_reg =
		(volatile uint32_t *)(base_addr + LED_OFFSET);

	// Set hex register to 0
	clear(hex_reg, led_reg);

	// Counter
	int8_t counter = MIN;

	while (1) {
		bool isPressed =
			false; // Boolean used for avoidng unnecessary access memory
		const uint32_t button_value =
			*btn_reg; // Current push button state

		if (button_value & BUT0_MASK) {
			if (counter < MAX) {
				counter++;
				isPressed = true;
			}
		} else if (button_value & BUT1_MASK) {
			if (counter > MIN) {
				counter--;
				isPressed = true;
			}
		} else {
			// Do nothing
		}

		if (isPressed) {
			// Split tens and units
			const uint8_t tens = counter / 10;
			const uint8_t unit = counter % 10;

			// Update value
			*hex_reg = (DEC_TO_HEX[tens] << 8) |
				       DEC_TO_HEX[unit];
			*led_reg = (1 << unit);
		}

		usleep(DELAY_US);
	}

	clear(hex_display, leds);

	munmap(base_addr, page_size);

	close(mem_fd);

	return 0;
}
