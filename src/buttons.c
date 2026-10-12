#include "modes.h"
#include <sys/types.h>
#include "nec.h"
#include "pico/stdlib.h"
#include "defaults.h"
#include "state.h"

typedef struct{
    uint pin;
    ButtonBehaviour on_ir_mode;
    ButtonBehaviour on_controller_mode; 
    uint32_t last_press_time; 
} RemoteButton;

static RemoteButton buttons[] = {
    { 
        .pin = BUTTON_1,
        .on_ir_mode  = { ACT_TOGGLE_MODE, 0 },
        .on_controller_mode = { ACT_TOGGLE_MODE, 0 }
    },
    { 
        .pin = BUTTON_2,
        .on_ir_mode  = { ACT_IR, 0x17 },        
        .on_controller_mode = { ACT_CONTROLLER, 0 }
    },
    { 
        .pin = BUTTON_3,
        .on_ir_mode  = { ACT_IR, 0x1A },
        .on_controller_mode = { ACT_CONTROLLER, 0 }
    },
    { 
        .pin = BUTTON_4,
        .on_ir_mode  = { ACT_IR, 0x1E },
        .on_controller_mode = { ACT_CONTROLLER, 0 }
    },
    { 
        .pin = BUTTON_5,
        .on_ir_mode  = { ACT_IR, 0x15 },
        .on_controller_mode = { ACT_CONTROLLER, 0}      
    }
};

#define NUM_BUTTONS (sizeof(buttons) / sizeof(buttons[0]))

static void handle_action(ButtonBehaviour *action){
    switch(action->type) {
        case ACT_TOGGLE_MODE: 
           toggle_mode();  
           break; 
        case ACT_IR: 
            send_nec(0x40, (uint8_t)action -> val);
            break; 

        case ACT_CONTROLLER: 
            break;
    }
}

static void handle_button_press_interrupt(uint pin, uint32_t events){
    if (!(events & GPIO_IRQ_EDGE_FALL)) return;
    uint32_t now = to_ms_since_boot(get_absolute_time());
    for (int i = 0; i < NUM_BUTTONS; i++) {
        if (buttons[i].pin == pin) {
            if (now - buttons[i].last_press_time > 250) {
                buttons[i].last_press_time = now;
                ButtonBehaviour *action = ( get_mode() == MODE_TV_IR) 
                    ? &buttons[i].on_ir_mode: &buttons[i].on_controller_mode;
                handle_action(action); 
            }
            break; 
        }
    }
}

void initialize_all_buttons() {
    for (int i = 0; i<NUM_BUTTONS; i++) {
        gpio_init(buttons[i].pin);
        gpio_set_dir(buttons[i].pin, GPIO_IN);
        gpio_pull_up(buttons[i].pin);
        buttons[i].last_press_time = 0;

        if(i==0) {
            gpio_set_irq_enabled_with_callback(buttons[0].pin, GPIO_IRQ_EDGE_FALL, 
                true, &handle_button_press_interrupt);
        } else {
            gpio_set_irq_enabled(buttons[i].pin, GPIO_IRQ_EDGE_FALL, true);
        }
    }
}
