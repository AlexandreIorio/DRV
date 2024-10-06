#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>
#include <sys/time.h>

struct {
		volatile uint32_t *reg;
} button_ctl;

/// @brief Method to initialize the button
/// @param button_register
void init_button(volatile uint32_t *button_register);

/// @brief Method to read the button
/// @param button_index
/// @return 1 if the button is pressed, 0 if not pressed and -1 if error
int read_button(uint8_t button_index);

/// @brief Method to check if a button is pressed for a certain duration
/// @param button_index the index of the button to check
/// @param duration the duration in ms
/// @return 0 if the button is pressed for the duration, -1 if the button relative to the index is not presssed and the duration in ms otherwise
int long_press(uint8_t button_index, unsigned duration_ms);

#endif // BUTTON_H