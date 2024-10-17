#include "switch.h"
#include "logger.h"

void init_switch(volatile uint32_t *button_register){
    logMessage(INFO, "Initializing switch\n");
    if (!button_register) {
        logMessage(WARNING, "switch register must be a valid address\n");
        return;
    }
    switch_ctl.reg = button_register;
}

int read_value(uint8_t switch_index) {
    if (!switch_ctl.reg) {
        logMessage(ERROR, "Error: switch register not initialized\n");
        return -1;
    }
    if (switch_index > 9) {
        logMessage(ERROR, "Error: invalid switch index\n");
        return -1;
    }

    int is_up = (*switch_ctl.reg & (1 << switch_index));
    return is_up;
}

uint16_t read_all_switches() {
    if (!switch_ctl.reg) {
        logMessage(ERROR, "Error: switch register not initialized\n");
        return -1;
    }

    uint16_t switches = *switch_ctl.reg;
    return switches;
}