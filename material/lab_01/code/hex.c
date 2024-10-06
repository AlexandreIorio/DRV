#include "hex.h"

static struct
{
    volatile uint32_t *reg; 
} hex_ctl;

void clear_hex(volatile uint32_t *hex_display, char display_index)
{
	int offset = display_index * HEX_DISPLAY_OFFSET;
	*hex_display = (DEC_TO_HEX[MIN] << offset);
}

void clear_all_hex(volatile uint32_t *hex_display, volatile uint32_t *leds)
{
    int len = sizeof(HEX_DISPLAY) / sizeof(HEX_DISPLAY[0]);

	for (int i = 0; i < len; i++)
    {
        clearHex(hex_display, i);
    }
}