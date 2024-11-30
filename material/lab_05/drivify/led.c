#include "led.h"
#include <linux/io.h>
#include <linux/printk.h>

#define LIB_NAME "led"
void led_up(uint8_t led_index, volatile void *__iomem reg)
{
	uint32_t led_value;
	if (!reg) {
		pr_err("[%s]: Led %d cannot be turned on because leds register not initialized\n",
		       LIB_NAME, led_index);
		return;
	}
	led_value = ioread32(reg);
	iowrite32(led_value | (1 << led_index), reg);
}

void led_down(uint8_t led_index, volatile void *__iomem reg)
{
	uint32_t led_value;
	if (!reg) {
		pr_err("[%s]: Led %d cannot be turned off because leds register not initialized\n",
		       LIB_NAME, led_index);
	}
	led_value = ioread32(reg);
	iowrite32(led_value & ~(1 << led_index), reg);
}

void clear_leds(volatile void *__iomem reg)
{
	if (!reg) {
		pr_err("[%s]: Led cannot be cleared because leds register not initialized\n",
		       LIB_NAME);
		return;
	}
	iowrite32(0, reg);
}

void all_leds_on(volatile void *__iomem reg)
{
	if (!reg) {
		pr_err("[%s]: Led cannot be turned on because leds register not initialized\n",
		       LIB_NAME);
		return;
	}
	iowrite32(0x000003FF, reg);
}
