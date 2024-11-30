#include "hex.h"
#include <linux/io.h>
#include <linux/printk.h>
#include <linux/log2.h>

#define LIB_NAME "hex"
static const uint8_t DIGITS[] = {
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
	0x77, // A
	0x7C, // B
	0x39, // C
	0x5E, // D
	0x79, // E
	0x71, // F
};

static int compute_num_digits(unsigned int value, unsigned int radix)
{
	unsigned int log2_value;
	unsigned int log2_radix;
	if (radix < 2 || radix > 16)
		return -EINVAL;

	if (value == 0)
		return 1;

	log2_value = ilog2(value + 1); // log2(value + 1)
	log2_radix = ilog2(radix); // log2(radix)

	return (log2_value + log2_radix - 1) / log2_radix;
}

void clear_hex_0_3(uint8_t display_index, volatile void *__iomem reg)
{
	uint32_t hex_value;
	if (display_index > 3) {
		pr_err("[%s]: Invalid hex display index\n", LIB_NAME);
		return;
	}
	if (!reg) {
		pr_err("[%s]: Hex display cannot be cleared because it is not initialized\n",
		       LIB_NAME);
		return;
	}
	hex_value = ioread32(reg);
	iowrite32(hex_value & ~(0xFF << (display_index * HEX_DISPLAY_OFFSET)),
		  reg);
}

void clear_hex_4_5(uint8_t display_index, volatile void *__iomem reg)
{
	uint32_t hex_value;
	if (display_index > 1) {
		pr_err("[%s]: Invalid hex display index\n", LIB_NAME);
		return;
	}
	if (!reg) {
		pr_err("[%s]: Hex display cannot be cleared because it is not initialized\n",
		       LIB_NAME);
		return;
	}
	hex_value = ioread32(reg);
	iowrite32(hex_value & ~(0xFF << (display_index * HEX_DISPLAY_OFFSET)),
		  reg);
}

void clear_all_hex_0_3(volatile void *__iomem reg)
{
	if (!reg) {
		pr_warn("[%s]: Hex display cannot be cleared because it is not initialized\n",
			LIB_NAME);
		return;
	}
	iowrite32(0, reg);
}

void clear_all_hex_4_5(volatile void *__iomem reg)
{
	if (!reg) {
		pr_warn("[%s]: Hex display cannot be cleared because it is not initialized\n",
			LIB_NAME);
		return;
	}
	iowrite32(0, reg);
}

void clear_all_hex(volatile void *__iomem reg_0_3,
		   volatile void *__iomem reg_4_5)
{
	clear_all_hex_0_3(reg_0_3);
	clear_all_hex_4_5(reg_4_5);
}

void all_hex_on(volatile void *__iomem reg_0_3, volatile void *__iomem reg_4_5)
{
	if (!reg_0_3 || !reg_4_5) {
		pr_warn("[%s]: Hex display cannot be turned on because it is not initialized\n",
			LIB_NAME);
		return;
	}
	iowrite32(0xFFFFFFFF, reg_0_3);
	iowrite32(0x0000FFFF, reg_4_5);
}

uint8_t get_decimal_digit(int number, uint8_t digit_index)
{
	while (digit_index > 0) {
		number /= 10;
		digit_index--;
	}
	return number % 10;
}

void display_digit_0_3(uint8_t number, uint8_t display_index,
		       volatile void *__iomem reg)
{
	uint32_t hex_value;
	if (display_index > 3) {
		pr_warn("[%s]: Invalid hex display index\n", LIB_NAME);
		return;
	}
	clear_hex_0_3(display_index, reg);
	hex_value = ioread32(reg);
	iowrite32(hex_value | DIGITS[number]
				      << (display_index * HEX_DISPLAY_OFFSET),
		  reg);
}

void display_digit_4_5(uint8_t number, uint8_t display_index,
		       volatile void *__iomem reg)
{
	uint32_t hex_value;
	if (display_index > 1) {
		pr_warn("[%s]: Invalid hex display index\n", LIB_NAME);
		return;
	}
	clear_hex_4_5(display_index, reg);
	hex_value = ioread32(reg);
	iowrite32(hex_value | DIGITS[number]
				      << (display_index * HEX_DISPLAY_OFFSET),
		  reg);
}

void display_number_0_3(int number, int radix, volatile void *__iomem reg)
{
	if (number < 0) {
		pr_err("[%s]: The number must be positive\n", LIB_NAME);
		return;
	}
	if (compute_num_digits(number, radix) > 4) {
		pr_err("[%s]: The number is too large to be displayed\n",
		       LIB_NAME);
		return;
	}

	for (int i = 0; i < 4; i++) {
		uint8_t digit = number % radix;
		if (number > 0 || i == 0) {
			display_digit_0_3(digit, i, reg);
		} else {
			clear_hex_0_3(i, reg);
		}
		number /= radix;
	}
}

void display_number_4_5(int number, int radix, volatile void *__iomem reg)
{
	if (number < 0) {
		pr_err("[%s]: The number must be positive\n", LIB_NAME);
		return;
	}
	if (compute_num_digits(number, radix) > 2) {
		pr_err("[%s]: The number is too large to be displayed\n",
		       LIB_NAME);
		return;
	}

	for (int i = 0; i < 2; i++) {
		uint8_t digit = number % radix;
		if (number > 0 || i == 0) {
			display_digit_4_5(digit, i, reg);
		} else {
			clear_hex_4_5(i, reg);
		}
		number /= radix;
	}
}

void display_number_on_displays(int value, int radix,
				volatile void *__iomem reg_0_3,
				volatile void *__iomem reg_4_5)
{
	int num_digits;
	int digit;
	value = abs(value);
	if (value == 0) {
		clear_all_hex(reg_0_3, reg_4_5);
		display_digit_0_3(0, 0, reg_0_3);
		display_digit_4_5(0, 0, reg_4_5);
		return;
	}
	if (radix < 2 || radix > 16) {
		pr_err("[%s]: Invalid radix\n", LIB_NAME);
		return;
	}

	if (value < 0) {
		pr_err("[%s]: The value must be positive\n", LIB_NAME);
		return;
	}

	num_digits = compute_num_digits(value, radix);
	for (int i = 0; i < 4; i++) {
		if (value / radix == 0 && i >= num_digits) {
			clear_hex_0_3(i, reg_0_3);
			continue;
		}
		digit = value % radix;
		if (value > 0 || i < num_digits) {
			display_digit_0_3(digit, i, reg_0_3);
		} else {
			clear_hex_0_3(i, reg_0_3);
		}
		value /= radix;
	}

	for (int i = 0; i < 2; i++) {
		if (value / radix == 0 && i + 4 >= num_digits) {
			clear_hex_4_5(i, reg_4_5);
			continue;
		}
		digit = value % radix;
		if (value > 0 || i + 4 < num_digits) {
			display_digit_4_5(digit, i, reg_4_5);
		} else {
			clear_hex_4_5(i, reg_4_5);
		}
		value /= radix;
	}
}

void display_time_3_0(int secondes, volatile void *__iomem reg)
{
	int minutes;
	if (secondes < 0) {
		pr_err("[%s]: The time must be positive\n", LIB_NAME);
		return;
	}

	minutes = secondes / 60;
	secondes %= 60;
	display_digit_0_3(secondes % 10, 0, reg);
	display_digit_0_3(secondes / 10, 1, reg);
	display_digit_0_3(minutes % 10, 2, reg);
	display_digit_0_3(minutes / 10, 3, reg);
}
