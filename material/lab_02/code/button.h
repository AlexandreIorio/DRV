#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>
#include <sys/time.h>

#define IRQ_ENABLE_OFFSET 0x8
#define IRQ_STATUS_OFFSET 0xC

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
/// @return 0 if the button is pressed for the duration, -1 if the button
/// relative to the index is not presssed and the duration in ms otherwise
int long_press(uint8_t button_index, unsigned duration_ms);

/// @brief Method to enable the interrupts for the buttons
/// @param button_mask the mask of the buttons to enable the interrupts
void button_enable_interrupts(uint8_t button_mask);

/// @brief Method to clear the interrupts for the buttons
/// @param button_mask the mask of the buttons to clear the interrupts
void button_clear_edge_reg_interrupts(uint8_t button_mask);

/// @brief Method to clear the interrupt for a specific button
/// @param button_index the index of the button to clear the interrupt
void button_clear_interrupt(uint8_t button_index);

/// @brief Method to read the status of the button interrupts
/// @param button_mask 
uint8_t button_status_interrupts(uint8_t button_mask);


#endif // BUTTON_H