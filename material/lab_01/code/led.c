#include "led.h"
#include <stdlib.h>
#include <stdbool.h>

void led_up(uint8_t led_index)
{
    if (!led_ctl.reg) {
        printf("Error: leds register not initialized\n");
        return EXIT_FAILURE;
    }
	*led_ctl.reg = *led_ctl.reg | (1 << led_index);
}

void led_down(uint8_t led_index)
{
    if (!led_ctl.reg) {
        printf("Error: leds register not initialized\n");
        return EXIT_FAILURE;
    }
	*led_ctl.reg = *led_ctl.reg & ~(1 << led_index);
}

void init_led(volatile uint32_t *led_register)
{
    if (!led_register) {
        printf("Error: leds register must be a valid address\n");
        return EXIT_FAILURE;
    }
    test_led();
    led_ctl.reg = led_register;
}

void test_led(void)
{
    if (!led_ctl.reg) {
        printf("Error: leds register not initialized\n");
        return EXIT_FAILURE;
    }

    int speed_us = 50000;
    int nb_round = 2;
    uint32_t led_state = 0x1; // initial state
    bool left_direction = true;

    int cycle = 0;
    while (cycle < nb_round * NUM_LEDS * 2) {
        *led_ctl.reg = led_state;

        usleep(speed_us);         

        if (left_direction) {
            
            led_state <<= 1;
            if (led_state & (1 << (NUM_LEDS - 1))) {
                left_direction = false; 
            }
        } else {
            
            led_state >>= 1;
            if (led_state & 0x1) {
                left_direction = true;
            }
        }

        cycle++;
    }
}

void clear_leds(void)
{
    if (!led_ctl.reg) {
        printf("Error: leds register not initialized\n");
        return EXIT_FAILURE;
    }
    *led_ctl.reg = 0x00000000;
}

void all_leds_on(void)
{
    if (!led_ctl.reg) {
        printf("Error: leds register not initialized\n");
        return EXIT_FAILURE;
    }
    *led_ctl.reg = 0x000003FF;
}

