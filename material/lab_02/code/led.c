#include "led.h"
#include <stdbool.h>
#include <stdlib.h>

void led_up(uint8_t led_index)
{
	if (!led_ctl.reg) {
		printf("Led %d cannot be turned on because leds register not initialized\n",
		       led_index);
		return;
	}
	*led_ctl.reg |= (1 << led_index);
}

void led_down(uint8_t led_index)
{
	if (!led_ctl.reg) {
		printf("Led %d cannot be turned off because leds register not initialized\n",
		       led_index);
		return;
	}
	*led_ctl.reg &= ~(1 << led_index);
}

int init_led(volatile uint32_t *led_register)
{
	printf("Initializing leds\n");
	if (!led_register) {
		printf("Error: leds register must be a valid address\n");
		return -1;
	}
	led_ctl.reg = led_register;
#if TEST_HW
	test_led();
#endif
	return 0;
}

void test_led(void)
{
	printf("Testing leds\n");
	if (!led_ctl.reg) {
		printf("Leds cannot be tested because leds register not initialized\n");
		return;
	}

	clear_leds();
	int speed_us = 50000;
	bool left_direction = true;

	int position = 0;
	do {
		if (left_direction) {
			position++;
			if (position == NUM_LEDS - 1) {
				left_direction = false;
			}

		} else {
			position--;
			if (position == 0) {
				left_direction = true;
			}
		}
		led_up(position);
		usleep(speed_us);
		led_down(position);
	} while (position != 0);
}

void clear_leds(void)
{
	if (!led_ctl.reg) {
		printf("Leds cannot be cleared because leds register not initialized\n");
		return;
	}
	*led_ctl.reg = 0x00000000;
}

void all_leds_on(void)
{
	if (!led_ctl.reg) {
		printf("Leds cannot be turned on because leds register not initialized\n");
		return;
	}
	*led_ctl.reg = 0x000003FF;
}