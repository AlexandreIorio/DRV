#include "keys.h"
#include "asm-generic/bug.h"
#include <linux/printk.h>
#include <linux/time.h>
#include <linux/io.h>

int read_key(void __iomem *key_reg, uint8_t key_index)
{
	uint8_t pressed;
	if (key_index > 3) {
		pr_err("[%s]: Impossible to read key %d because it's out of range\n",
		       HW_NAME, key_index);
		return -1;
	}
	pressed = read_keys(key_reg) & (1 << key_index);
	return pressed;
}

uint8_t read_keys(void __iomem *key_reg)
{
	void __iomem *edge_capture_reg;
	if (!key_reg) {
		pr_err("[%s]: Impossible to read keys register because it is NULL\n",
		       HW_NAME);
		return -1;
	}

	edge_capture_reg = key_reg + KEYS_EDGE_OFFSET;
	return ioread8(edge_capture_reg);
}

void keys_enable_interrupts(void __iomem *key_reg, uint8_t key_mask)
{
	void *irq_mask_reg;
	pr_info("[%s]: Enabling interrupts for keys 0x%x\n", HW_NAME, key_mask);
	BUG_ON(!key_reg);

	irq_mask_reg = key_reg + KEYS_IRQ_OFFSET;
	iowrite8(key_mask, irq_mask_reg);
	keys_clear_edge_reg(key_reg, key_mask);
	pr_info("[%s]: Interrupts enabled for keys 0x%x\n", HW_NAME, key_mask);
}

void keys_disable_interrupts(void __iomem *key_reg)
{
	void *irq_mask_reg;
	pr_info("[%s]: Disabling interrupts for keys\n", HW_NAME);
	BUG_ON(!key_reg);

	irq_mask_reg = key_reg + KEYS_IRQ_OFFSET;
	iowrite8(0, irq_mask_reg);
	pr_info("[%s]: Interrupts disabled for keys\n", HW_NAME);
}

void keys_clear_edge_reg(void __iomem *key_reg, uint8_t key_mask)
{
	void __iomem *edge_capture_reg;
	BUG_ON(!key_reg);
	printk("[%s]: Clearing keys interrupts 0x%x\n", HW_NAME, key_mask);
	edge_capture_reg = key_reg + KEYS_EDGE_OFFSET;
	pr_info("[%s]: Clearing keys interrupts 0x%x\n", HW_NAME, key_mask);
	iowrite8(key_mask, edge_capture_reg);
	pr_info("[%s]: Interrupts keys cleared 0x%x\n", HW_NAME, key_mask);
}

// void keys_clear_interrupt(void __iomem *key_reg, uint8_t key_mask)
// {
// 	void __iomem *edge_capture_reg;
// 	BUG_ON(!key_reg);
// 	edge_capture_reg = key_reg + KEYS_EDGE_OFFSET;
// 	pr_info("[%s]: Clearing keys interrupt mask x%x\n", HW_NAME, key_mask);
// 	iowrite8(key_mask, edge_capture_reg);
// 	pr_info("[%s]: Interrupts keys cleared mask 0x%x\n", HW_NAME, key_mask);
// }

uint8_t key_status_interrupts(void __iomem *key_reg, uint8_t key_mask)
{
	void __iomem *edge_capture_mask_reg;
	uint8_t key_status;

	BUG_ON(!key_reg);
	edge_capture_mask_reg = key_reg + KEYS_EDGE_OFFSET;

	pr_info("[%s]: Reading keys interrupts status\n", HW_NAME);

	key_status = ioread8(edge_capture_mask_reg) & key_mask;
	return key_status;
}
