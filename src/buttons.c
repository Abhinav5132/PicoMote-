#include "buttons.h"
#include "modes.h"
#include <hardware/gpio.h>
#include <pico/time.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include "nec.h"
#include "pico/stdlib.h"
#include "defaults.h"
#include "state.h"
#include "hardware/adc.h"

typedef struct {
    int16_t x;
    int16_t y; 
} JoystickReading; 

typedef struct {
    int pin;
    ButtonBehaviour on_ir_mode;
    ButtonBehaviour on_controller_mode; 
    uint32_t last_press_time; 
    bool is_range; 
} RemoteButton;

static RemoteButton buttons[] = {
    {   
        // mode switch
        .pin = BUTTON_1,
        .on_ir_mode  = { ACT_TOGGLE_MODE, 0 },
        .on_controller_mode = { ACT_TOGGLE_MODE, 0 }
    },
    { 
        //power button
        .pin = BUTTON_2,
        .on_ir_mode  = { ACT_IR, 0x17 },        
        .on_controller_mode = { ACT_CONTROLLER, 0 }
    },
    { 
        //Vol+
        .pin = BUTTON_3,
        .on_ir_mode  = { ACT_IR, 0x1A },
        .on_controller_mode = { ACT_CONTROLLER, 0 }
    },
    { 
        //Vol-
        .pin = BUTTON_4,
        .on_ir_mode  = { ACT_IR, 0x1B },
        .on_controller_mode = { ACT_CONTROLLER, 0 }
    },
    { 
        //Menu
        .pin = BUTTON_5,
        .on_ir_mode  = { ACT_IR, 0x15 },
        .on_controller_mode = { ACT_CONTROLLER, 0}
    },
    {   
        //OK
        .pin = JOYSTICK_BUTTON,
        .on_ir_mode = {ACT_IR, 0x16},
        .on_controller_mode = {ACT_CONTROLLER, 0}
    },
};

#define NUM_BUTTONS (sizeof(buttons) / sizeof(buttons[0]))

JoystickReading read_joystick(void) {
    adc_select_input(JOYSTICK_X);
    uint16_t raw_x = adc_read();

    adc_select_input(JOYSTICK_Y);
    uint16_t raw_y = adc_read();

    int16_t norm_x = (int16_t)raw_x - 2048;
    int16_t norm_y = (int16_t)raw_y - 2048; 

    JoystickReading reading; 
    reading.x = (abs(norm_x) > deadzone_x) ? norm_x : 0; 
    reading.y = (abs(norm_y) > deadzone_y) ? norm_y : 0; 

    return reading; 
}

void poll_joystick(void) {
    if (get_sleep_state() == SLEEP) return;
    
    JoystickReading reading = read_joystick(); 
    if (reading.x != 0 || reading.y != 0){
        wake(); 
        if(reading.x > 0){
            send_nec(0x40, 0x0A);
        } else if(reading.x < 0) {
            send_nec(0x40, 0x1B);   
        }
        
        if (reading.y > 0){
            send_nec(0x40, 0x18);
        } else if(reading.y < 0){
            send_nec(0x40, 0x19);
        }
    
        sleep_ms(300);

    }
}

void handle_action(ButtonBehaviour *action){
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
    wake();
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

void initialize_all_buttons(void) {
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
    gpio_init(JOYSTICK_POWER);
    gpio_set_dir(JOYSTICK_POWER, GPIO_OUT);
    gpio_put(JOYSTICK_POWER, 1);
    adc_init();
}

void buttons_enable_dormant_wake(bool enable) {
    for (int i = 0; i < NUM_BUTTONS; i++) {
        gpio_set_dormant_irq_enabled(buttons[i].pin, 
            IO_BANK0_DORMANT_WAKE_INTE0_GPIO0_LEVEL_LOW_BITS, enable); 
    }
}
