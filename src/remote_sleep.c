#include "remote_sleep.h"
#include "defaults/defaults.h"
#include "modes.h"
#include "state.h"
#include "buttons.h"
#include "pico/low_power.h"
#include "pico/aon_timer.h"
#include "hardware/structs/powman.h"
#include "hardware/powman.h"
#include "hardware/gpio.h"
#include <pico/stdio.h>
#include <pico/time.h>
#include <pico/util/queue.h>
#include <stdbool.h>
#include <stdio.h>

void save_state_to_powman(RemoteMode mode) {
    powman_hw->scratch[0] = (uint32_t)mode; 
}

RemoteMode restore_state_from_powman(void) {
    return (RemoteMode)powman_hw->scratch[0]; 
}

void enter_pstate(void) {
    save_state_to_powman(get_mode()); 

    powman_enable_gpio_wakeup(1, BUTTON_1, false, false); 
    powman_enable_gpio_wakeup(2, BUTTON_2, false, false); 
    powman_enable_gpio_wakeup(3, JOYSTICK_BUTTON, true, false); 
    pstate_bitset_t pstate = pstate_bitset_none(); 
    low_power_pstate_until_gpio_pin_state(BUTTON_3, false, false, &pstate, NULL);
}

void enter_dormant_sleep(void) {
    gpio_put(JOYSTICK_POWER, 0); 
    buttons_enable_dormant_wake(true);
    sleep();
}

void return_from_dormant_sleep(void) {
    buttons_enable_dormant_wake(false);
    gpio_put(JOYSTICK_POWER, 1);
}

void manage_power(void) {
    uint32_t now = to_ms_64_since_boot(get_absolute_time()); 

    if (now - get_last_active_time() <= DORMANT_TIMEOUT) return;
    if (!is_joystick_neutral() || !gpio_get(JOYSTICK_BUTTON)) return;
    if (!is_button_queue_empty()) return; 
    
    enter_dormant_sleep();
    absolute_time_t deadline = aon_timer_make_timeout_time_ms(PSTATE_TIMEOUT);
    low_power_dormant_until_aon_timer(deadline, DORMANT_CLOCK_SOURCE_LPOSC, NULL); 

    return_from_dormant_sleep(); 
    stdio_init_all();
    printf("returned from sleep"); 
    if (absolute_time_diff_us(aon_timer_get_absolute_time(), deadline) <= 0) {
        // Woken up by the AON timer timeout go to P-state sleep
        gpio_put(JOYSTICK_POWER, 1);
        sleep_ms(100);
        gpio_put(JOYSTICK_POWER, 0);
        sleep_ms(100);
        gpio_put(JOYSTICK_POWER, 1);
        sleep_ms(100); 
        gpio_put(JOYSTICK_POWER, 0);
        sleep_ms(100);
        enter_pstate();
    } else {
        // Woken up by a button press
        wake(); 
    }
}
