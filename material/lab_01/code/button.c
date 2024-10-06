#include "button.h"
#include <stdio.h>

void init_button(volatile uint32_t *button_register)
{
		printf("Initializing button\n");
		if (!button_register) {
				printf("Error: button register must be a valid address\n");
				return;
		}

		button_ctl.reg = button_register;
}

int read_button(uint8_t button_index)
{
		if (!button_ctl.reg) {
				printf("Button %d cannot be read because button register not initialized\n",
					   button_index);
				return -1;
		}

		if (button_index > 3) {
				printf("The index %d is invalid, it must be between 0 and 3\n",
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
