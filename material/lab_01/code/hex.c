#include <math.h>
#include "hex.h"

int init_hex(volatile uint32_t *hex_register_0_3,
	     volatile uint32_t *hex_register_4_5)
{
	printf("Initializing hex display\n");
	if (!hex_register_0_3 || !hex_register_4_5) {
		printf("Error: hex display register must be a valid address\n");
		return -1;
	}

	hex_ctl.reg_0_3 = hex_register_0_3;
	hex_ctl.reg_4_5 = hex_register_4_5;

	test_hex();
}

void clear_hex(uint8_t display_index)
{
	if (!hex_ctl.reg_0_3 || !hex_ctl.reg_4_5) {
		printf("Hex display %d cannot be cleared because it is not initialized\n",
		       display_index);
		return;
	}
    if (display_index < 4) {
        *hex_ctl.reg_0_3 &= ~(0xFF << (display_index * HEX_DISPLAY_OFFSET));
    } else {
        *hex_ctl.reg_4_5 &= ~(0xFF << ((display_index - 4) * HEX_DISPLAY_OFFSET));
    }
}

void clear_all_hex()
{
	if (!hex_ctl.reg_0_3 || !hex_ctl.reg_4_5) {
		printf("Hex display cannot be cleared because it is not initialized\n");
		return;
	}

	*hex_ctl.reg_0_3 = 0x00000000;
	*hex_ctl.reg_4_5 = 0x00000000;
}

// volatile uint32_t *get_hex_reg(uint8_t display_index)
// {
// 	if (display_index > 5) {
// 		printf("Error: invalid hex display index\n");
// 		return 0;
// 	}

// 	if (!hex_ctl.reg_0_3 || !hex_ctl.reg_4_5) {
// 		printf("Error: hex display register not initialized\n");
// 		return 0;
// 	}

// 	volatile uint32_t *base_addr = display_index < 4 ? hex_ctl.reg_0_3 :
// 							   hex_ctl.reg_4_5;
// 	int offset;

// 	if (display_index < 4) {
// 		offset = display_index * HEX_DISPLAY_OFFSET;
// 	} else {
// 		offset = (display_index - 4) * HEX_DISPLAY_OFFSET;
// 	}

// 	return (volatile uint32_t *)(base_addr + offset);
// }

void test_hex(void)
{
	printf("Testing hex display\n");
	if (!hex_ctl.reg_0_3 || !hex_ctl.reg_4_5) {
		printf("Error: hex display register not initialized\n");
		return;
	}

	all_hex_on();
	usleep(500000);
	clear_all_hex();
}

void all_hex_on(void)
{
	if (!hex_ctl.reg_0_3 || !hex_ctl.reg_4_5) {
		printf("Hex display cannot be turned on because it is not initialized\n");
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

void display_digit(uint8_t number, uint8_t display_index) {
    clear_hex(display_index);
    if (display_index > 5) {
        printf("Error: invalid hex display index\n");
        return;
    }

    if (display_index < 4) {
        *hex_ctl.reg_0_3 |= DEC_TO_HEX[number] << (display_index * HEX_DISPLAY_OFFSET);
    } else {
        *hex_ctl.reg_4_5 |= DEC_TO_HEX[number] << ((display_index - 4) * HEX_DISPLAY_OFFSET);
    }
}

void display_decimal_number(int number)
{
    if (number < 0) {
        printf("The number must be positive\n");
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
