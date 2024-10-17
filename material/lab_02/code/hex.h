#ifndef HEX_H
#define HEX_H

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>

#define HEX_DISPLAY_OFFSET 0x08
#define NUM_HEX_DISPLAY 6
#define TEST_HW 1

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

/// @brief Initialize the hex display
/// @param hex_register_0_3 the register for the first 4 hex display
/// @param hex_register_4_5 the register for the last 2 hex display
/// @return 0 if the initialization is successful, -1 otherwise
int init_hex(volatile uint32_t *hex_register_0_3,
	     volatile uint32_t *hex_register_4_5);

/// @brief Clear the hex display
void clear_hex(uint8_t display_index);

/// @brief Clear all hex displays
void clear_all_hex(void);

/// @brief Test the hex display
void test_hex(void);

/// @brief Turn on all the hex display
void all_hex_on(void);

/// @brief Method to get the decimal digit at a specific index
/// @param number The number to convert
/// @param digit_index The index of the digit to get
/// @return the decimal digit at the specified index
uint8_t get_decimal_digit(int number, uint8_t digit_index);

/// @brief Method to display a number on a specific display
/// @param number The number to display
/// @param display_index The index of the display
void display_digit(uint8_t number, uint8_t display_index);

/// @brief Method to display a decimal number on a specific display
/// @param number The number to display
void display_decimal_number(int number);

/// @brief Method to display a value on the hex displays
/// @param value The value to display
void display_value_on_displays(int value, int radix);

#endif // HEX_H
