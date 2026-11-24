#include "debug/debug_log.h"
#include <pico/time.h>
#include <stdint.h>
#include <stdio.h>
#include "ir/nec.h"
#include "pico/stdlib.h"
#include "pico/low_power.h"
#include "hardware/structs/powman.h"
#include "buttons.h"
#include "defaults/defaults.h"
#include "state.h"
#include "remote_sleep.h"
#include "joystick.h"
#include "debug/test_nec.h"

int main() {
    stdio_init_all();
    if (nec_tx_init(pio0, IR_TRANSMITTER) < 0) {
        panic("[ERROR] NEC initialization failed");
    }
    initialize_all_buttons();
    initialize_joystick(); 
    calibrate_joystick(JOYSTICK_CALIBRATION_SAMPLES); 
    if (powman_hw->chip_reset & POWMAN_CHIP_RESET_HAD_SWCORE_PD_BITS) {
        debug_log("[INFO] Restarted after P-state sleep\n");
        set_mode(restore_state_from_powman());
    }

    low_power_start_aon_timer();
    wake();
    debug_log("[INFO] Finished initialization.\n");
    while (true) {
        process_button_events(); 
        poll_joystick();
        if (DEBUG) debug_log_poll();
        manage_power();
    }
    return 0; 
}
