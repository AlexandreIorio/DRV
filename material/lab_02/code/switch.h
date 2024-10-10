#ifndef SWITCH_H
#define SWITCH_H

#include <stdint.h>
#include <sys/time.h>

struct {
  volatile uint32_t *reg;
} switch_ctl;

/// @brief Method to initialize the button
/// @param button_register
void init_switch(volatile uint32_t *button_register);

/// @brief Method to read the value of a switch
/// @param switch_index the index of the button to check
/// @return 1 if the switch is up, 0 if is down and -1 if error
int read_value(uint8_t switch_index);

/// @brief Method to read all the switches
/// @return the value of all the switches
int read_all_switches();

#endif // SWITCH_H