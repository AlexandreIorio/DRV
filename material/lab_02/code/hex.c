#include "hex.h"
#include <stdlib.h>
#include <stdbool.h>
#include "logger.h"

static struct {
	volatile uint32_t *reg_0_3;
	volatile uint32_t *reg_4_5;
} hex_ctl;

static const uint8_t DEC_TO_HEX[] = {
	0x3F, // 0
	0x06, // 1
	0x5B, // 2
	0x4F, // 3
	0x66, // 4
	0x6D, // 5
	0x7D, // 6
	0x07, // 7
	0x7F, // 8
	0x6F, // 9
};

int init_hex(volatile uint32_t *hex_register_0_3,
	     volatile uint32_t *hex_register_4_5)
{
	logMessage(INFO, "Initializing hex display\n");
	if (!hex_register_0_3 || !hex_register_4_5) {
		logMessage(
			ERROR,
			"Error: hex display register must be a valid address\n");
		return -1;
	}

	hex_ctl.reg_0_3 = hex_register_0_3;
	hex_ctl.reg_4_5 = hex_register_4_5;
#if TEST_HW
	test_hex();
#endif
	return 0;
}

void clear_hex(uint8_t display_index)
{
	if (!hex_ctl.reg_0_3 || !hex_ctl.reg_4_5) {
		logMessage(
			WARNING,
			"Hex display %d cannot be cleared because it is not initialized\n",
			display_index);
		return;
	}
	if (display_index < 4) {
		*hex_ctl.reg_0_3 &=
			~(0xFF << (display_index * HEX_DISPLAY_OFFSET));
	} else {
		*hex_ctl.reg_4_5 &=
			~(0xFF << ((display_index - 4) * HEX_DISPLAY_OFFSET));
	}
}

void clear_all_hex()
{
	if (!hex_ctl.reg_0_3 || !hex_ctl.reg_4_5) {
		logMessage(
			WARNING,
			"Hex display cannot be cleared because it is not initialized\n");
		return;
	}

	*hex_ctl.reg_0_3 = 0x00000000;
	*hex_ctl.reg_4_5 = 0x00000000;
}

void test_hex(void)
{
	logMessage(INFO, "Testing hex display\n");
	if (!hex_ctl.reg_0_3 || !hex_ctl.reg_4_5) {
		logMessage(WARNING, "hex display register not initialized\n");
		return;
	}
	int display_index = 0;
	bool left_direction = true;
	do {
		if (left_direction) {
			display_index++;
			if (display_index == NUM_HEX_DISPLAY - 1) {
				left_direction = false;
			}

		} else {
			display_index--;
			if (display_index == 0) {
				left_direction = true;
			}
		}
		display_digit(8, display_index);
		usleep(50000);
		clear_hex(display_index);
	} while (display_index > 0);
	logMessage(INFO, "Hex display test finished\n");
}

void all_hex_on(void)
{
	if (!hex_ctl.reg_0_3 || !hex_ctl.reg_4_5) {
		logMessage(WARNING, "Hex display cannot be turned on because it is not initialized\n");
		return;
	}
	*hex_ctl.reg_0_3 = 0xFFFFFFFF;
	*hex_ctl.reg_4_5 = 0x0000FFFF;
}

uint8_t get_decimal_digit(int number, uint8_t digit_index)
{
	while (digit_index > 0) {
		number /= 10;
		digit_index--;
	}
	return number % 10;
}

void display_digit(uint8_t number, uint8_t display_index)
{
	clear_hex(display_index);
	if (display_index > 5) {
		logMessage(WARNING, "invalid hex display index\n");
		return;
	}

	if (display_index < 4) {
		*hex_ctl.reg_0_3 |= DEC_TO_HEX[number]
				    << (display_index * HEX_DISPLAY_OFFSET);
	} else {
		*hex_ctl.reg_4_5 |= DEC_TO_HEX[number] << ((display_index - 4) *
							   HEX_DISPLAY_OFFSET);
	}
}

void display_decimal_number(int number)
{
	if (number < 0) {
		logMessage(WARNING, "The number must be positive\n");
		return;
	}

	for (int i = 0; i < NUM_HEX_DISPLAY; i++) {
		uint8_t digit = number % 10;
		if (number > 0 || i == 0) {
			display_digit(digit, i);
		} else {
			clear_hex(i);
		}
		number /= 10;
	}
}

void display_value_on_displays(int value, int radix)
{
    value = abs(value);
    if (value == 0) {
        clear_all_hex();
        display_digit(0, 0);
        return;
    }
	if (radix < 2 || radix > 16) {
		logMessage(WARNING, "invalid radix\n");
		return;
	}

	if (value < 0) {
		logMessage(WARNING, "The value must be positive\n");
		return;
	}

	int num_digits = (int)ceil(log(value + 1) / log(radix));
	for (int i = 0; i < NUM_HEX_DISPLAY; i++) {
		if (value / radix == 0 && i >= num_digits) {
			clear_hex(i);
			continue;
		}
		int digit = value % radix;
		if (value > 0 || i < num_digits) {
			display_digit(digit, i);
		} else {
			clear_hex(i);
		}
		value /= radix;
	}
}
