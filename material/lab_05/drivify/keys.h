#ifndef key_H
#define key_H

#include <linux/types.h>
#include <linux/module.h>

#define KEYS_OFFSET 0x50
#define KEYS_MASK 0x0F
#define KEYS_IRQ_OFFSET 0x8
#define KEYS_EDGE_OFFSET 0xC

#define HW_NAME "keys"

/// @brief Method to read the key
/// @param key_reg the register of the key
/// @param key_index
/// @return 1 if the key is pressed, 0 if not pressed and -1 if error
int read_key(void __iomem *key_reg, uint8_t key_index);

/// @brief Method to read the keys
/// @param key_reg the register of the key
/// @return the status of the keys
uint8_t read_keys(void __iomem *key_reg);

/// @brief Method to enable the interrupts for the keys
/// @param key_reg the register of the key
/// @param key_mask the mask of the keys to enable the interrupts
void keys_enable_interrupts(void __iomem *key_reg, uint8_t key_mask);

/// @brief Method to disable the interrupts for the all keys
void keys_disable_interrupts(void __iomem *key_reg);

/// @brief Method to clear the interrupts for the keys
/// @param key_reg the register of the key
/// @param key_mask the mask of the keys to clear the interrupts
void keys_clear_edge_reg(void __iomem *key_reg, uint8_t key_mask);

/// @brief Method to clear the interrupt for a specific key
/// @param key_reg the register of the key
/// @param key_mask the mask of the key to clear the interrupt
// void keys_clear_interrupt(void __iomem *key_reg, uint8_t key_mask);

/// @brief Method to read the status of the key interrupts
/// @param key_reg the register of the key
/// @param key_mask
uint8_t key_status_interrupts(void __iomem *key_reg, uint8_t key_mask);

#endif // key_H
