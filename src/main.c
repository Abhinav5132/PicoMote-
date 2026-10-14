#include <hardware/gpio.h>
#include <stdint.h>
#include "nec.h"
#include "buttons.h"
#include "defaults.h"
#include "pico/stdlib.h"

int main() {
    stdio_init_all();
    init_ir(IR_TRANSMITTER);
    initialize_all_buttons();

    while (true) {
        poll_joystick();
    }
    return 0; 
}
