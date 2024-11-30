#ifndef HEX_H
#define HEX_H

#include <linux/kernel.h>

#define HEX_DISPLAY_OFFSET 0x08
#define NUM_HEX_DISPLAY 6
#define TEST_HW 1

/// @brief Initialize the hex display
/// @param hex_register_0_3 the register for the first 4 hex display
/// @param hex_register_4_5 the register for the last 2 hex display
/// @return 0 if the initialization is successful, -1 otherwise
int init_hex(volatile uint32_t *hex_register_0_3,
	     volatile uint32_t *hex_register_4_5);

/// @brief Clear the hex display at index 0 to 3
/// @param display_index the index of the display to clear
/// @param reg the register of the display
void clear_hex_0_3(uint8_t display_index, volatile void *__iomem reg);

/// @brief Clear the hex display at index 4 to 5
/// @param display_index the index of the display to clear
/// @param reg the register of the display
void clear_hex_4_5(uint8_t display_index, volatile void *__iomem reg);
/// @brief Clear all hex displays

/// @brief Clear all hex displays
/// @param reg the register of the display
void clear_all_hex_0_3(volatile void *__iomem reg);

/// @brief Clear all hex displays
/// @param reg the register of the display
void clear_all_hex_4_5(volatile void *__iomem reg);

/// @param reg_0_3 the register for the first 4 hex display
/// @param reg_4_5 the register for the last 2 hex display
void clear_all_hex(volatile void *__iomem reg_0_3,
		   volatile void *__iomem reg_4_5);

/// @brief Turn on all the hex display
/// @param reg_0_3 the register for the first 4 hex display
/// @param reg_4_5 the register for the last 2 hex display
void all_hex_on(volatile void *__iomem reg_0_3, volatile void *__iomem reg_4_5);

/// @brief Method to display a digit on a specific display
/// @param number The number to display
/// @param display_index The index of the display
/// @param reg The register of the display
void display_digit_0_3(uint8_t number, uint8_t display_index,
		       volatile void *__iomem reg);

/// @brief Method to display a digit on a specific display
/// @param number The number to display
/// @param display_index The index of the display
/// @param reg The register of the display
void display_digit_4_5(uint8_t number, uint8_t display_index,
		       volatile void *__iomem reg);

/// @brief Method to get the decimal digit at a specific index
/// @param number The number to convert
/// @param digit_index The index of the digit to get
/// @return the decimal digit at the specified index
uint8_t get_decimal_digit(int number, uint8_t digit_index);

/// @brief Method to display a decimal number on a specific display
/// @param number The number to display
/// @param radix The radix of the number
/// @param reg The register of the display
void display_number_0_3(int number, int radix, volatile void *__iomem reg);

/// @brief Method to display a decimal number on a specific display
/// @param number The number to display
/// @param radix The radix of the number
/// @param reg The register of the display
void display_number_4_5(int number, int radix, volatile void *__iomem reg);

/// @brief Method to display a value on the hex displays
/// @param value The value to display
/// @param radix The radix of the value
/// @param reg_0_3 The register for the first 4 hex display
void display_number_on_displays(int value, int radix,
				volatile void *__iomem reg_0_3,
				volatile void *__iomem reg_4_5);

/// @brief Method to display a time on the hex displays
/// @param secondes The time in secondes
/// @param reg The register of the display
void display_time_3_0(int secondes, volatile void *__iomem reg);
#endif // HEX_H
