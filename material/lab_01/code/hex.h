#ifndef HEX_H
#define HEX_H

#define HEX_OFFSET 0x20
#define HEX_DISPLAY_OFFSET 0x08

#define MIN 0
#define MAX 99

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

static const uint8_t HEX_DISPLAY[] = {
	0,
	1,
};


#include <stdint.h>

/// @brief Clear the hex display
void clear_hex(uint8_t display_index);

/// @brief Clear all hex displays
void clear_all_hex(void);

#endif // HEX_H
