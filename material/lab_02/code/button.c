#include "button.h"
#include "logger.h"
#include <stdio.h>

static struct {
	volatile uint32_t *reg;
	volatile uint32_t *irq_mask_reg;
	volatile uint32_t *edge_capture_mask_reg;
} button_ctl;

void init_button(volatile uint32_t *button_register)
{
	logMessage(DEBUG,"Initializing button\n");
	if (!button_register) {
		logMessage(ERROR, "Error: button register must be a valid address\n");
		return;
	}

	button_ctl.reg = button_register;
    logMessage(DEBUG, "Button initialized at addr 0x%p\n", button_register);
	button_ctl.irq_mask_reg = (volatile uint32_t*)((uint32_t)button_ctl.reg | IRQ_ENABLE_OFFSET);
    button_ctl.edge_capture_mask_reg = (volatile uint32_t*)((uint32_t)button_ctl.reg | IRQ_STATUS_OFFSET);
}

int read_button(uint8_t button_index)
{
	if (!button_ctl.reg) {
		logMessage(ERROR, "Button %d cannot be read because button register not initialized\n",
		       button_index);
		return -1;
	}

	if (button_index > 3) {
		logMessage(ERROR, "The index %d is invalid, it must be between 0 and 3\n",
		       button_index);
		return -1;
	}
	int pressed = (*button_ctl.reg & (1 << button_index));
	return pressed;
}

int long_press(uint8_t button_index, unsigned duration_ms)
{
	static int pressed = 0;
	static struct timeval pressed_time;
	static int pressed_button_index = -1;

	if (read_button(button_index) && pressed == 0) {
		pressed = 1;
		pressed_button_index = button_index;
		gettimeofday(&pressed_time, NULL);
	}

	if (pressed_button_index == button_index && read_button(button_index)) {
		struct timeval current_time;
		gettimeofday(&current_time, NULL);

		long elapsed_time =
			(current_time.tv_sec - pressed_time.tv_sec) * 1000;
		elapsed_time +=
			(current_time.tv_usec - pressed_time.tv_usec) / 1000;

		if (elapsed_time > duration_ms) {
			return 0;
		}

		return duration_ms - elapsed_time;
	}

	if (pressed_button_index == button_index &&
	    !read_button(button_index) && pressed) {
		pressed = 0;
		pressed_button_index = -1;
	}

	return -1;
}

void button_enable_interrupts(uint8_t button_mask)
{
    logMessage(DEBUG, "Enabling buttons interrupts\n");
	if (!button_ctl.irq_mask_reg) {
        logMessage(ERROR,"Error: button irq mask register not initialized\n");
        return;
    }
    *button_ctl.irq_mask_reg = button_mask;
    button_clear_interrupts(button_mask);
    logMessage(DEBUG, "Interrupts buttons enabled at addr %p with mask %x\n", button_ctl.irq_mask_reg, *button_ctl.irq_mask_reg  );
}

void button_clear_interrupts(uint8_t button_mask)
{
    logMessage(DEBUG, "Clearing buttons interrupts 0x%x\n", button_mask);
    if (!button_ctl.edge_capture_mask_reg) {
        logMessage(ERROR,"Error: button edge capture mask register not initialized\n");
        return;
    }
    *button_ctl.edge_capture_mask_reg = button_mask;
    logMessage(DEBUG, "Interrupts buttons cleared 0x%x\n", button_mask);
}

void button_clear_interrupt(uint8_t button_index)
{
    logMessage(DEBUG, "Clearing buttons interrupt at index %d\n", button_index);
    if (!button_ctl.edge_capture_mask_reg) {
        logMessage(ERROR,"Error: button edge capture mask register not initialized\n");
        return;
    }
    *button_ctl.edge_capture_mask_reg = (1 << button_index);
    logMessage(DEBUG, "Interrupts buttons cleared at index %d\n", button_index);
}

uint8_t button_status_interrupts(uint8_t button_mask){
    logMessage(DEBUG, "Reading buttons interrupts status\n");
    if (!button_ctl.edge_capture_mask_reg) {
        logMessage(ERROR,"Error: button edge capture mask register not initialized\n");
        return -1;
    }
    uint8_t button_status = *button_ctl.edge_capture_mask_reg & button_mask;

    return button_status;
}
