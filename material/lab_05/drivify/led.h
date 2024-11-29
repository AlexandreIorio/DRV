#ifndef LED_H
#define LED_H

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#define LED_OFFSET 0x00
#define NUM_LEDS 10
#define TEST_HW 1

/// @brief Method to turn on the LED
/// @param led_index The index of the LED to turn on
void led_up(uint8_t led_index);

/// @brief Method to turn off the LED
/// @param led_index The index of the LED to turn off
void led_down(uint8_t led_index);

/// @brief Method to initialize the LED register and test the LED
/// @param led_register
/// @return 0 if the initialization is successful, -1 otherwise
int init_led(volatile uint32_t *led_register);

/// @brief Methode to test the LED
void test_led(void);

/// @brief Methode to clear the LED
void clear_leds(void);

/// @brief Methode to turn on all the LED
void all_leds_on(void);

#endif // LED_H