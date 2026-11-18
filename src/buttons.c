#include "debug_log.h"
#include "buttons.h"
#include "ir/nec.h"
#include "modes.h"
#include <hardware/gpio.h>
#include <pico/time.h>
#include <pico/types.h>
#include <pico/util/queue.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include "defaults/defaults.h"
#include "state.h"
#include "defaults/ir_defaults.h"

typedef struct {
    uint32_t pressed_us;
    Action action; 
    RemoteMode mode;  
} ButtonEvent; 

static volatile uint32_t dropped_events;
static queue_t button_queque;

typedef struct {
    uint pin; 
    Action action;  
    uint32_t last_press_time; 
} RemoteButton;

static RemoteButton buttons[] = {
    { .pin = BUTTON_1,        .action = ACTION_TOGGLE_MODE },
    { .pin = BUTTON_2,        .action = ACTION_POWER },
    { .pin = BUTTON_3,        .action = ACTION_VOLUME_UP },
    { .pin = BUTTON_4,        .action = ACTION_VOLUME_DOWN },
    { .pin = BUTTON_5,        .action = ACTION_MENU },
    { .pin = JOYSTICK_BUTTON, .action = ACTION_CONFIRM },
};

static const uint8_t pin_to_button[] = {
    [BUTTON_1]        = 1,
    [BUTTON_2]        = 2,
    [BUTTON_3]        = 3,
    [BUTTON_4]        = 4,
    [BUTTON_5]        = 5,
    [JOYSTICK_BUTTON] = 6,
};


#define NUM_BUTTONS (sizeof(buttons) / sizeof(buttons[0]))


bool handle_ir_action(Action action) {

    if (action == ACTION_TOGGLE_MODE) {
        toggle_mode(); 
        return true; 
    }
    
    uint8_t command; 
    switch(action){
        case ACTION_POWER: command = TV_IR_POWER; break; 
        case ACTION_VOLUME_UP: command = TV_IR_VOLUME_UP; break; 
        case ACTION_VOLUME_DOWN: command = TV_IR_VOLUME_DOWN; break; 
        case ACTION_MENU: command = TV_IR_MENU; break; 
        case ACTION_CONFIRM: command = TV_IR_CONFIRM; break;
        case ACTION_TOGGLE_MODE: break;
        case ACTION_UNBOUND: break; 
    }

    return send_nec(TV_IR_ADDRESS, command); 
}

bool handle_hid_action(Action action) {
    //TODO 
    if (action == ACTION_TOGGLE_MODE) {
        toggle_mode(); 
        return true; 
    }
    return false; 
}

bool try_handle_action(Action action, RemoteMode mode){
    switch(mode) {
        case MODE_TV_IR: 
            if (!handle_ir_action(action)) return false; 
            break;

        case MODE_AIR_MOUSE: 
            if (!handle_hid_action(action)) return false; 
            break; 
    }
    return true; 
}

static void interrupt_handle_button_press(uint pin, uint32_t events){
    if (!(events & GPIO_IRQ_EDGE_FALL)) return;
    if (pin >= sizeof(pin_to_button) || pin_to_button[pin] == 0) return;

    uint index = pin_to_button[pin] - 1;
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (now - buttons[index].last_press_time < BUTTON_DEBOUNCE_MS) return;
    buttons[index].last_press_time = now;

    Action action = buttons[index].action;
    RemoteMode mode = get_mode();
    ButtonEvent event = {
        .pressed_us = time_us_32(),
        .action = action, 
        .mode = mode,
    }; 

    if (!queue_try_add(&button_queque, &event)) ++dropped_events;
}

void process_button_events(void) {
    ButtonEvent event;
    if (!queue_try_remove(&button_queque, &event)) return;

    uint32_t delay_us = time_us_32() - event.pressed_us;
    wake();
    for (int i = 0; i < BUTTON_RETRIES; i++) {
        if (try_handle_action(event.action, event.mode)) {
            debug_log("[INFO] Handled %s, mode: %s, irq_us=%lu, dispatch_delay_us=%lu, queue_drops=%lu\n",
                action_name(event.action), mode_name(event.mode), (unsigned long)event.pressed_us,
                (unsigned long)delay_us, (unsigned long)dropped_events);
            return;
        }
        debug_log("[ERROR] Failed to handle action: %s, mode: %s\n",
            action_name(event.action), mode_name(event.mode));
    }
}

bool is_button_queue_empty(void){
    return queue_is_empty(&button_queque); 
}

void initialize_all_buttons(void) {
    
    queue_init(&button_queque, sizeof(ButtonEvent), BUTTON_QUEUE_SIZE);
    for (int i = 0; i<NUM_BUTTONS; i++) {
        gpio_init(buttons[i].pin);
        gpio_set_dir(buttons[i].pin, GPIO_IN);
        gpio_pull_up(buttons[i].pin);
        buttons[i].last_press_time = to_ms_since_boot(get_absolute_time()) - BUTTON_DEBOUNCE_MS;

        if(i==0) {
            gpio_set_irq_enabled_with_callback(buttons[0].pin, GPIO_IRQ_EDGE_FALL, 
                true, &interrupt_handle_button_press);
        } else {
            gpio_set_irq_enabled(buttons[i].pin, GPIO_IRQ_EDGE_FALL, true);
        }
    }
    sleep_ms(10); 
}

void buttons_enable_dormant_wake(bool enable) {
    for (int i = 0; i < NUM_BUTTONS; i++) {
        gpio_set_dormant_irq_enabled(buttons[i].pin, 
            IO_BANK0_DORMANT_WAKE_INTE0_GPIO0_LEVEL_LOW_BITS, enable); 
    }
}
