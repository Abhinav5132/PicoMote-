#include <hardware/structs/io_bank0.h>
#include <pico/time.h>
#include <stdint.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "nec.h"

//IMU
#define SCL 16
#define SDA 17

//IR RECEIVER 
#define IR_TRANSMITTER 18

//CONTROL BUTTONS 
#define BUTTON_1 19
#define BUTTON_2 20
#define BUTTON_3 21
#define BUTTON_4 22

//joystick 
#define JOYSTICK_X 3
#define JOYSTICK_Y 2
#define JOYSTICK_BUTTON 4

typedef struct{
    uint pin;
    uint8_t cmd;
    uint32_t last_press_time; 
} RemoteButton;

static RemoteButton buttons[] = {
    {.pin = BUTTON_1, .cmd=0x17},
    {.pin = BUTTON_2, .cmd=0x1A},
    {.pin = BUTTON_3, .cmd=0x1E},
    {.pin = BUTTON_4, .cmd=0x15}
    
};

#define NUM_BUTTONS sizeof(buttons) / sizeof(buttons[0])

static void handle_button_press_interrupt(uint pin, uint32_t events){
    if (!(events & GPIO_IRQ_EDGE_FALL)) return;
    uint32_t now = to_ms_since_boot(get_absolute_time());
    for (int i = 0; i<NUM_BUTTONS; i++) {
        if((buttons[i].pin == pin) && 
            (now - buttons[i].last_press_time > 250)){
            buttons[i].last_press_time = now;
            send_nec(0x40, buttons[i].cmd);
        } else {
            continue;
        }
    }
}

static void inline initialize_all_buttons() {
    for (int i = 0; i<NUM_BUTTONS; i++) {
        gpio_init(buttons[i].pin);
        gpio_set_dir(buttons[i].pin, GPIO_IN);
        gpio_pull_up(buttons[i].pin);
        buttons[i].last_press_time = 0;

        if(i==0) {
            gpio_set_irq_enabled_with_callback(buttons[0].pin, GPIO_IRQ_EDGE_FALL, true, &handle_button_press_interrupt);
        } else {
            gpio_set_irq_enabled(buttons[i].pin, GPIO_IRQ_EDGE_FALL, true);
        }
    }
}

int main() {
    stdio_init_all();
    init_ir(IR_TRANSMITTER);
    initialize_all_buttons();
    while(true);
    return 0; 
}