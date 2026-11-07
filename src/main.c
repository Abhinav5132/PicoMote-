#include <pico/time.h>
#include <stdint.h>
#include "ir/nec.h"
#include "pico/stdlib.h"
#include "pico/low_power.h"
#include "hardware/structs/powman.h"
#include "buttons.h"
#include "defaults/defaults.h"
#include "state.h"
#include "remote_sleep.h"

int main() {
    stdio_init_all();
    if (nec_tx_init(pio0, IR_TRANSMITTER) < 0) {
        panic("NEC initialization failed");
    }
    initialize_all_buttons();
    calibrate_joystick(JOYSTICK_CALIBRATION_SAMPLES); 
    if (powman_hw->chip_reset & POWMAN_CHIP_RESET_HAD_SWCORE_PD_BITS) {
        set_mode(restore_state_from_powman());
    }

    low_power_start_aon_timer();
    wake();

    while (true) {
        process_button_events(); 
        poll_joystick();
        manage_power();
    }
    return 0; 
}
