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
	volatile uint32_t *hex_display = get_hex_reg(display_index);
	if (!hex_display) {
		printf("Hex display %d cannot be cleared because it is not initialized\n", display_index);
		return;
	}
	*hex_display = 0x00;
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

volatile uint32_t *get_hex_reg(uint8_t display_index)
{
    if (display_index > 5) {
        printf("Error: invalid hex display index\n");
        return 0;
    }

	if (!hex_ctl.reg_0_3 || !hex_ctl.reg_4_5) {
		printf("Error: hex display register not initialized\n");
		return 0;
	}

	volatile uint32_t *base_addr = display_index < 4 ? hex_ctl.reg_0_3 :
						  hex_ctl.reg_4_5;
	int offset;

	if (display_index < 4) {
		offset = display_index * HEX_DISPLAY_OFFSET;
	} else {
		offset = (display_index - 4) * HEX_DISPLAY_OFFSET;
	}

	return (volatile uint32_t*)base_addr + offset;
}

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