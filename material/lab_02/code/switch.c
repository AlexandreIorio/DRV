#include "switch.h"

void init_switch(volatile uint32_t *button_register){
    printf("Initializing switch\n");
    if (!button_register) {
        printf("Error: switch register must be a valid address\n");
        return;
    }
    switch_ctl.reg = button_register;
}

int read_value(uint8_t switch_index) {
    if (!switch_ctl.reg) {
        printf("Error: switch register not initialized\n");
        return -1;
    }
    if (switch_index > 9) {
        printf("Error: invalid switch index\n");
        return -1;
    }

    int is_up = (*switch_ctl.reg & (1 << switch_index));
    return is_up;
}

int read_all_switches() {
    if (!switch_ctl.reg) {
        printf("Error: switch register not initialized\n");
        return -1;
    }

    int switches = *switch_ctl.reg;
    return switches;
}