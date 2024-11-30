#ifndef LED_H
#define LED_H
#include <linux/init.h>

#define LED_OFFSET 0x00
#define LED_MASK 0x000003FF
#define NUM_LEDS 10
#define TEST_HW 1

/// @brief Method to turn on the LED
/// @param led_index The index of the LED to turn on
/// @param reg The register of the LED
void led_up(uint8_t led_index, volatile void *__iomem reg);

/// @brief Method to turn off the LED
/// @param led_index The index of the LED to turn off
/// @param reg The register of the LED
void led_down(uint8_t led_index, volatile void *__iomem reg);

/// @brief Methode to clear the LED
/// @param reg The register of the LED
void clear_leds(volatile void *__iomem reg);

/// @brief Methode to turn on all the LED
/// @param reg The register of the LED
void all_leds_on(volatile void *__iomem reg);

#endif // LED_H
