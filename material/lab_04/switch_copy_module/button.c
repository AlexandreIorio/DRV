#include "button.h"
#include "asm-generic/bug.h"
#include <linux/printk.h>
#include <linux/time.h>
#include <linux/io.h>

int read_button(void __iomem *button_reg, uint8_t button_index)
{
	void __iomem *edge_capture_reg;
	uint8_t pressed;
	if (!button_reg) {
		pr_err("Button %d cannot be read because button register not initialized\n",
		       button_index);
		return -1;
	}

	edge_capture_reg = (uint8_t *)button_reg + IRQ_STATUS_OFFSET;

	if (button_index > 3) {
		pr_err("The index %d is invalid, it must be between 0 and 3\n",
		       button_index);
		return -1;
	}
	pressed = ioread8(edge_capture_reg) & (1 << button_index);
	return pressed;
}

void button_enable_interrupts(void __iomem *button_reg, uint8_t button_mask)
{
	void *irq_mask_reg;
	pr_info("Enabling interrupts for buttons 0x%x\n", button_mask);
	BUG_ON(!button_reg);

	irq_mask_reg = (uint8_t *)button_reg + IRQ_ENABLE_OFFSET;
	iowrite8(button_mask, irq_mask_reg);
	button_clear_edge_reg(button_reg, button_mask);
	pr_info("Interrupts enabled for buttons 0x%x\n", button_mask);
}

void button_clear_edge_reg(void __iomem *button_reg, uint8_t button_mask)
{
	void __iomem *edge_capture_reg;
	BUG_ON(!button_reg);
	printk("Clearing buttons interrupts 0x%x\n", button_mask);
	edge_capture_reg = (uint8_t *)button_reg + IRQ_STATUS_OFFSET;
	pr_info("Clearing buttons interrupts 0x%x\n", button_mask);
	iowrite8(button_mask, edge_capture_reg);
	pr_info("Interrupts buttons cleared 0x%x\n", button_mask);
}

void button_clear_interrupt(void __iomem *button_reg, uint8_t button_index)
{
	void __iomem *edge_capture_reg;
	BUG_ON(!button_reg);
	edge_capture_reg = (uint8_t *)button_reg + IRQ_STATUS_OFFSET;
	pr_info("Clearing buttons interrupt at index %d\n", button_index);

	iowrite8(1 << button_index, edge_capture_reg);
	pr_info("Interrupts buttons cleared at index %d\n", button_index);
}

uint8_t button_status_interrupts(void __iomem *button_reg, uint8_t button_mask)
{
	void __iomem *edge_capture_mask_reg;
	uint8_t button_status;

	BUG_ON(!button_reg);
	edge_capture_mask_reg = (uint8_t *)button_reg + IRQ_STATUS_OFFSET;

	pr_info("Reading buttons interrupts status\n");

	button_status = ioread8(edge_capture_mask_reg) & button_mask;
	return button_status;
}
