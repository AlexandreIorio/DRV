#ifndef BUTTON_H
#define BUTTON_H

#include <linux/types.h>
#include <linux/module.h>

#define IRQ_ENABLE_OFFSET 0x8
#define IRQ_STATUS_OFFSET 0xC

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alexandre Iorio");

/// @brief Method to read the button
/// @param button_reg the register of the button
/// @param button_index
/// @return 1 if the button is pressed, 0 if not pressed and -1 if error
int read_button(void __iomem *button_reg, uint8_t button_index);

/// @brief Method to enable the interrupts for the buttons
/// @param button_reg the register of the button
/// @param button_mask the mask of the buttons to enable the interrupts
void button_enable_interrupts(void __iomem *button_reg, uint8_t button_mask);

/// @brief Method to clear the interrupts for the buttons
/// @param button_reg the register of the button
/// @param button_mask the mask of the buttons to clear the interrupts
void button_clear_edge_reg(void __iomem *button_reg, uint8_t button_mask);

/// @brief Method to clear the interrupt for a specific button
/// @param button_reg the register of the button
/// @param button_index the index of the button to clear the interrupt
void button_clear_interrupt(void __iomem *button_reg, uint8_t button_index);

/// @brief Method to read the status of the button interrupts
/// @param button_reg the register of the button
/// @param button_mask
uint8_t button_status_interrupts(void __iomem *button_reg, uint8_t button_mask);

EXPORT_SYMBOL(read_button);
EXPORT_SYMBOL(button_enable_interrupts);
EXPORT_SYMBOL(button_clear_edge_reg);
EXPORT_SYMBOL(button_clear_interrupt);
EXPORT_SYMBOL(button_status_interrupts);

#endif // BUTTON_H
