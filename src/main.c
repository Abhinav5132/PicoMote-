#include <stdint.h>
#include "pico/stdlib.h"
#include "pico/low_power.h"
#include "hardware/structs/powman.h"
#include "nec.h"
#include "buttons.h"
#include "defaults.h"
#include "state.h"
#include "remote_sleep.h"

int main() {
    stdio_init_all();
    init_ir(IR_TRANSMITTER);
    initialize_all_buttons();

    if (powman_hw->chip_reset & POWMAN_CHIP_RESET_HAD_SWCORE_PD_BITS) {
        set_mode(restore_state_from_powman());
    }

    low_power_start_aon_timer();
    wake();

    while (true) {
        poll_joystick();
        manage_power();
    }
    return 0; 
}
