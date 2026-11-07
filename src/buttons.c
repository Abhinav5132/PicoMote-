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
#include <stdlib.h>
#include <sys/types.h>
#include "pico/stdlib.h"
#include "defaults/defaults.h"
#include "state.h"
#include "hardware/adc.h"
#include "defaults/ir_defaults.h"

typedef struct {
    int16_t x;
    int16_t y; 
} JoystickReading; 

typedef struct {
    int pin;
    Action action; 
    uint32_t last_press_time; 
    bool is_range; 
} RemoteButton;

typedef struct {
    uint8_t button_index; 
    uint32_t timestamp_ms;    
} ButtonEvent; 

static queue_t button_quque; 

static RemoteButton buttons[] = {
    {   
        // mode switch
        .pin = BUTTON_1,
        .action = ACTION_TOGGLE_MODE
    },
    { 
        //power button
        .pin = BUTTON_2,
        .action = ACTION_POWER
    },
    { 
        //Vol+
        .pin = BUTTON_3,
        .action = ACTION_VOLUME_UP
    },
    { 
        //Vol-
        .pin = BUTTON_4,
        .action = ACTION_VOLUME_DOWN
    },
    { 
        //Menu
        .pin = BUTTON_5,
        .action = ACTION_MENU
    },
    {   
        //OK
        .pin = JOYSTICK_BUTTON,
        .action = ACTION_CONFIRM
    },
};

#define NUM_BUTTONS (sizeof(buttons) / sizeof(buttons[0]))

void calibrate_joystick(int samples) {
    int32_t sx = 0, sy = 0; 
    for(int i = 0; i < samples; i++) {
        adc_select_input(JOYSTICK_X); 
        sx += adc_read(); 
        adc_select_input(JOYSTICK_Y); 
        sy += adc_read(); 
        sleep_ms(2); 
    }

    joy_center_x = (int16_t)(sx / samples); 
    joy_center_y = (int16_t)(sy / samples); 
}

JoystickReading read_joystick(void) {
    if (get_sleep_state() == SLEEP || !gpio_get(JOYSTICK_POWER)){
        return (JoystickReading){0, 0}; 
    }
    adc_select_input(JOYSTICK_X);
    uint16_t raw_x = adc_read();

    adc_select_input(JOYSTICK_Y);
    uint16_t raw_y = adc_read();

    int16_t norm_x = (int16_t)raw_x - joy_center_x;
    int16_t norm_y = (int16_t)raw_y - joy_center_y; 

    JoystickReading reading; 
    reading.x = (abs(norm_x) > deadzone_x) ? norm_x : 0; 
    reading.y = (abs(norm_y) > deadzone_y) ? norm_y : 0; 

    return reading; 
}

void poll_joystick(void) {
    if (get_sleep_state() == SLEEP) return;

    static uint32_t last_poll_time = 0; 
    static uint32_t last_repeat_time = 0; 

    uint32_t now = to_ms_since_boot(get_absolute_time()); 
    if (now - last_poll_time < 20) return; 
    last_poll_time = now; 
    
    JoystickReading reading = read_joystick(); 
    if (reading.x != 0 || reading.y != 0){ 
        wake(); 
        if(reading.x > 0){
            //send_nec(0x40, 0x0A);
        } else if(reading.x < 0) {
            //send_nec(0x40, 0x1B);   
        }
        
        if (reading.y > 0){
            //send_nec(0x40, 0x18);
        } else if(reading.y < 0){
            //send_nec(0x40, 0x19);
        }
    }
}

bool is_joystick_neutral() {
    JoystickReading reading = read_joystick(); 
    if (reading.x != 0 || reading.y != 0) {
        return false; 
    }
    return true; 
}

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

bool try_handle_action(Action action){
    switch(get_mode()) {
        case MODE_TV_IR: 
            if (!handle_ir_action(action)) return false; 
            break;

        case MODE_AIR_MOUSE: 
            if (!handle_hid_action(action)) return false; 
            break; 
    }
    return true; 
}

/*Maybe the interrupt function should only queue the pin and what the remote state was at that time to keep interrupt lean  */
static void interrupt_handle_button_press(uint pin, uint32_t events){
    if (!(events & GPIO_IRQ_EDGE_FALL)) return;
    wake();
    uint32_t now = to_ms_since_boot(get_absolute_time());
    for (int i = 0; i < NUM_BUTTONS; i++) {
        if (buttons[i].pin != pin) continue; 
        if ((uint32_t)(now - buttons[i].last_press_time < 256)) return; 

        buttons[i].last_press_time = now; 
        
        ButtonEvent event = {
            .button_index = (uint8_t)i,
            .timestamp_ms = now,
        }; 
        if (get_mode() == MODE_AIR_MOUSE)  {
            //TODO no queue events, would be handled immediately 
        } else {
            if (!queue_try_add(&button_quque, &event)){
                printf("Button queue exceeded.");
            } 
        }
       
        return; 
    }
}

void process_button_events(void) {
     ButtonEvent event; 
     if (queue_try_peek(&button_quque, &event)) {
         Action action = buttons[event.button_index].action; 
         if(try_handle_action(action)) {
             queue_try_remove(&button_quque, &event); 
         } else {
             printf("Failed to process button event %d", action); 
             queue_try_remove(&button_quque, &event); // ADD gracefull error handling here later 
         }
     }
}

bool is_button_queue_empty(void){
    return queue_is_empty(&button_quque); 
}

void initialize_all_buttons(void) {
    gpio_init(JOYSTICK_POWER);
    gpio_set_dir(JOYSTICK_POWER, GPIO_OUT);
    gpio_put(JOYSTICK_POWER, 1);
    adc_init();
    adc_gpio_init(JOYSTICK_X_GPIO);
    adc_gpio_init(JOYSTICK_Y_GPIO);
    queue_init(&button_quque, sizeof(ButtonEvent), BUTTON_QUEUE_SIZE);
    for (int i = 0; i<NUM_BUTTONS; i++) {
        gpio_init(buttons[i].pin);
        gpio_set_dir(buttons[i].pin, GPIO_IN);
        gpio_pull_up(buttons[i].pin);
        buttons[i].last_press_time = 0;

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
