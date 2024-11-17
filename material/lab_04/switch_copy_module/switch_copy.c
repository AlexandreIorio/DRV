#include "linux/printk.h"
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/of.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("REDS");
MODULE_DESCRIPTION("Introduction to the interrupt and platform drivers");

#define BUTTON_OFFSET 0x50
#define BUTTON_MASK 0x0F
#define BUTTON_IRQ_OFFSET 0x8
#define BUTTON_EDGE_OFFSET 0xC
#define BUTTON0 0x01
#define BUTTON1 0x02

#define SWITCH_OFFSET 0x40
#define SWITCHES_MASK 0x3FF

#define SEG1_OFFSET 0x20
#define SEG2_OFFSET 0x30
#define SEG_DISPLAY_OFFSET 0x08
#define SEG_NB_DISPLAY 6

#define BASE_ADDRESS 0xFF200000

#define MAX_VALUE 999999

static const uint8_t DEC_TO_HEX[] = {
	0x3F, // 0
	0x06, // 1
	0x5B, // 2
	0x4F, // 3
	0x66, // 4
	0x6D, // 5
	0x7D, // 6
	0x07, // 7
	0x7F, // 8
	0x6F, // 9
};

struct priv {
	void __iomem *button_reg;
	void __iomem *switch_reg;
	void __iomem *seg1_reg;
	void __iomem *seg2_reg;
	uint32_t cnt;
};

/// NOTES:
/// Normalement les methodes ci-dessous devraient etre dans un fichier header
/// mais je n'ai pas reussi a inclure le fichier header dans le module sans avoir
/// de probleme.

/// @brief Method to enable the interrupts for the buttons
/// @param button_reg the register of the button
/// @param button_mask the mask of the buttons to enable the interrupts
void button_enable_interrupts(void __iomem *button_reg, uint8_t button_mask);

/// @brief Method to clear the interrupts for the buttons
/// @param button_reg the register of the button
/// @param button_mask the mask of the buttons to clear the interrupts
void button_clear_edge_reg(void __iomem *button_reg, uint8_t button_mask);

/// @brief Method to read the status of the button interrupts
/// @param button_reg the register of the button
/// @param button_mask
uint8_t button_status_interrupts(void __iomem *button_reg, uint8_t button_mask);

/// @brief Method to disable the interrupts for the buttons
/// @param button_reg the register of the button
/// @param button_mask the mask of the buttons to disable the interrupts
void button_disable_interrupts(void __iomem *button_reg, uint8_t button_mask);

/// @brief Method to read all the switches
/// @return the mask of active switches (1 if up, 0 if down)
uint16_t read_all_switches(void __iomem *switch_reg);

/// @brief Display a digit on the 7-segment display
/// @param seg_reg_0 The register of the first 7-segment display
/// @param seg_reg_1 The register of the second 7-segment display
/// @param value The value to display
static void display_digit_on_seg(void __iomem *seg_reg_0,
				 void __iomem *seg_reg_1, uint32_t value);

/// @brief Clear all the 7-segment displays
/// @param hex_reg The register of the 7-segment displays
static void clear_all_hex(void __iomem *hex_reg);

static irqreturn_t irq_handler(int irq, void *dev_id)
{
	struct priv *priv;
	uint8_t button_status;
	pr_info("Interrupt received\n");
	priv = (struct priv *)dev_id;
	BUG_ON(!priv);

	// read the status of the button interrupts
	button_status = button_status_interrupts(priv->button_reg, BUTTON_MASK);

	if (button_status & BUTTON0) {
		pr_info("Button 0 pressed\n");
		priv->cnt++;
		display_digit_on_seg(priv->seg1_reg, priv->seg2_reg, priv->cnt);
	} else if (button_status & BUTTON1) {
		priv->cnt = read_all_switches(priv->switch_reg);
		display_digit_on_seg(priv->seg1_reg, priv->seg2_reg, priv->cnt);
	}
	button_clear_edge_reg(priv->button_reg, BUTTON_MASK);
	return IRQ_HANDLED;
}

static int switch_copy_probe(struct platform_device *pdev)
{
	struct priv *priv;
	struct resource *resource;
	int ret;
	int irq;

	void __iomem *base_address;
	pr_info("Switch copy: driver registering\n");
	priv = devm_kzalloc(&pdev->dev, sizeof(struct priv), GFP_KERNEL);
	BUG_ON(!priv);

	platform_set_drvdata(pdev, priv);

	resource = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	BUG_ON(!resource);

	// get the base address of the device
	base_address = devm_ioremap_resource(&pdev->dev, resource);
	BUG_ON(IS_ERR(base_address));

	priv->button_reg = (uint8_t *)base_address + BUTTON_OFFSET;
	priv->switch_reg = (uint8_t *)base_address + SWITCH_OFFSET;
	priv->seg1_reg = (uint8_t *)base_address + SEG1_OFFSET;
	priv->seg2_reg = (uint8_t *)base_address + SEG2_OFFSET;

	// get the IRQ number
	irq = platform_get_irq(pdev, 0);
	BUG_ON(irq < 0);

	//set the IRQ handler
	ret = devm_request_irq(&pdev->dev, irq, irq_handler, IRQF_SHARED,
			       "switch_copy", (void *)priv);
	BUG_ON(ret);

	// enable the interrupts
	button_enable_interrupts(priv->button_reg, BUTTON_MASK);
	// clear current catched rised edges by setting to 1 the edge capture register
	button_clear_edge_reg(priv->button_reg, BUTTON_MASK);

	// Initialize the 7-segment display to 0
	display_digit_on_seg(priv->seg1_reg, priv->seg2_reg, 0);
	pr_info("Switch copy: driver registered\n");
	return 0;
}

static int switch_copy_remove(struct platform_device *pdev)
{
	struct priv *priv;

	pr_info("Switch copy: driver unregistering\n");
	priv = platform_get_drvdata(pdev);
	BUG_ON(!priv);

	clear_all_hex(priv->seg1_reg);
	clear_all_hex(priv->seg2_reg);
	button_disable_interrupts(priv->button_reg, BUTTON_MASK);
	kfree(priv);
	pr_info("Switch copy: driver unregistered\n");
	return 0;
}

/**************************BUTTONS**************************/

void button_enable_interrupts(void __iomem *button_reg, uint8_t button_mask)
{
	void *irq_mask_reg;
	pr_info("Enabling interrupts for buttons 0x%x\n", button_mask);
	BUG_ON(!button_reg);

	irq_mask_reg = (uint8_t *)button_reg + BUTTON_IRQ_OFFSET;
	iowrite8(button_mask, irq_mask_reg);
	button_clear_edge_reg(button_reg, button_mask);
	pr_info("Interrupts enabled for buttons 0x%x\n", button_mask);
}

void button_disable_interrupts(void __iomem *button_reg, uint8_t button_mask)
{
	void *irq_mask_reg;
	pr_info("Disabling interrupts for buttons 0x%x\n", button_mask);
	BUG_ON(!button_reg);

	irq_mask_reg = (uint8_t *)button_reg + BUTTON_IRQ_OFFSET;
	iowrite8(0, irq_mask_reg);
	button_clear_edge_reg(button_reg, button_mask);
	pr_info("Interrupts disabled for buttons 0x%x\n", button_mask);
}

void button_clear_edge_reg(void __iomem *button_reg, uint8_t button_mask)
{
	void __iomem *edge_capture_reg;
	BUG_ON(!button_reg);
	printk("Clearing buttons interrupts 0x%x\n", button_mask);
	edge_capture_reg = (uint8_t *)button_reg + BUTTON_EDGE_OFFSET;
	pr_info("Clearing buttons interrupts 0x%x\n", button_mask);
	iowrite8(button_mask, edge_capture_reg);
	pr_info("Interrupts buttons cleared 0x%x\n", button_mask);
}

uint8_t button_status_interrupts(void __iomem *button_reg, uint8_t button_mask)
{
	void __iomem *edge_capture_mask_reg;
	uint8_t button_status;

	BUG_ON(!button_reg);
	edge_capture_mask_reg = (uint8_t *)button_reg + BUTTON_EDGE_OFFSET;

	pr_info("Reading buttons interrupts status\n");

	button_status = ioread8(edge_capture_mask_reg) & button_mask;
	return button_status;
}

/**************************SWITCHES**************************/

uint16_t read_all_switches(void __iomem *switch_reg)
{
	uint16_t switches = ioread16(switch_reg) & SWITCHES_MASK;
	return switches;
}

/**************************HEX DISPLAY**************************/

void clear_all_hex(void __iomem *hex_reg)
{
	iowrite32(0, hex_reg);
}

static void display_digit_on_seg(void __iomem *seg_reg_0,
				 void __iomem *seg_reg_1, uint32_t value)
{
	uint8_t unit, ten, hundred, thousand, ten_thousand, hundred_thousand;
	uint32_t seg0_value, seg1_value;

	if (value > MAX_VALUE) {
		value = MAX_VALUE;
	}

	unit = value % 10;
	ten = (value / 10) % 10;
	hundred = (value / 100) % 10;
	thousand = (value / 1000) % 10;
	ten_thousand = (value / 10000) % 10;
	hundred_thousand = (value / 100000) % 10;

	seg0_value = DEC_TO_HEX[unit] |
		     (DEC_TO_HEX[ten] << SEG_DISPLAY_OFFSET) |
		     (DEC_TO_HEX[hundred] << (SEG_DISPLAY_OFFSET * 2)) |
		     (DEC_TO_HEX[thousand] << (SEG_DISPLAY_OFFSET * 3));

	seg1_value = DEC_TO_HEX[ten_thousand] |
		     (DEC_TO_HEX[hundred_thousand] << SEG_DISPLAY_OFFSET);

	iowrite32(seg0_value, seg_reg_0);
	iowrite32(seg1_value, seg_reg_1);
}

static const struct of_device_id switch_copy_driver_id[] = {
	{ .compatible = "drv2024" },
	{ /* END */ },
};

MODULE_DEVICE_TABLE(of, switch_copy_driver_id);

static struct platform_driver switch_copy_driver = {
	.driver = {
		.name = "drv-lab4",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(switch_copy_driver_id),
	},
	.probe = switch_copy_probe,
	.remove = switch_copy_remove,
};

module_platform_driver(switch_copy_driver);
