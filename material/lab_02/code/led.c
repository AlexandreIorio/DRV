#include "led.h"
#include <stdbool.h>
#include <stdlib.h>
#include "logger.h"

static struct {
	volatile uint32_t *reg;
} led_ctl;

void led_up(uint8_t led_index)
{
    logMessage(DEBUG, "Turning on led %d\n", led_index);
	if (!led_ctl.reg) {
		logMessage(WARNING, "Led %d cannot be turned on because leds register not initialized\n",
		       led_index);
		return;
	}
	*led_ctl.reg |= (1 << led_index);
}

void led_down(uint8_t led_index)
{
    logMessage(DEBUG, "Turning off led %d\n", led_index);
	if (!led_ctl.reg) {
		logMessage(WARNING, "Led %d cannot be turned on because leds register not initialized\n",
		       led_index);
		return;
	}
	*led_ctl.reg &= ~(1 << led_index);
}

int init_led(volatile uint32_t *led_register)
{
	logMessage(INFO, "Initializing leds\n");
	if (!led_register) {
		logMessage(ERROR, "Error: leds register must be a valid address\n");
		return -1;
	}
	led_ctl.reg = led_register;
#if TEST_HW
    logMessage(INFO, "Testing leds enabled\n");
	test_led();
#endif
	return 0;
}

void test_led(void)
{
	logMessage(INFO, "Testing leds\n");
	if (!led_ctl.reg) {
		logMessage(WARNING, "Leds cannot be tested because leds register not initialized\n");
		return;
	}
    logMessage(DEBUG, "Clearing leds\n");
	clear_leds();
    logMessage(DEBUG, "Turning on all leds\n");
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
    logMessage(DEBUG, "Clearing leds\n");
	if (!led_ctl.reg) {
		logMessage(WARNING, "Leds cannot be cleared because leds register not initialized\n");
		return;
	}
	*led_ctl.reg = 0x00000000;
}

void all_leds_on(void)
{
	if (!led_ctl.reg) {
		logMessage(WARNING, "Leds cannot be cleared because leds register not initialized\n");
		return;
	}
	*led_ctl.reg = 0x000003FF;
}
