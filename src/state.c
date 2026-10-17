#include "state.h"
#include "defaults.h"
#include "modes.h"
#include <hardware/gpio.h>
#include <pico/platform/sections.h>
#include <pico/time.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// when the remote is not in use it should go to sleep. 
// when in sleep no pooling occurs and only when the remote is brought into a awake state
// 
// use this: 
// hardware pin to turn off power to the joystick. 
// and we can use the MDU to figure out if the remote has been picked up or not 


typedef struct{
    RemoteMode mode; 
    RemoteSleepState current_sleep_state; 
    uint32_t last_activity_time; 
} RemoteState; 

static __persistent_data(RemoteState) current_state = 
    {.mode = MODE_TV_IR, .current_sleep_state=AWAKE };

RemoteMode get_mode(void) {
    return current_state.mode;
}

void toggle_mode(void) {
    current_state.mode = (current_state.mode == MODE_TV_IR) ? MODE_AIR_MOUSE : MODE_TV_IR;
}

void set_mode(RemoteMode mode) {
    current_state.mode = mode;
}

RemoteSleepState get_sleep_state(void) {
    return current_state.current_sleep_state;
}

void sleep(void) {
    if (current_state.current_sleep_state == AWAKE) {
        current_state.current_sleep_state = SLEEP;
        gpio_put(JOYSTICK_POWER, 0); 
    }
}

void wake(void){
    if (current_state.current_sleep_state == SLEEP) {
        current_state.current_sleep_state = AWAKE;
        gpio_put(JOYSTICK_POWER, 1); 
    }
    current_state.last_activity_time = to_ms_since_boot(get_absolute_time());
}

bool should_sleep(uint32_t timeout_ms) {
    if (current_state.current_sleep_state == SLEEP) return true; 
    uint32_t now = to_ms_since_boot(get_absolute_time()); 
    return(now - current_state.last_activity_time > timeout_ms); 
}

uint32_t get_last_active_time() {
    return current_state.last_activity_time; 
}