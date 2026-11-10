#include "defaults/defaults.h"
#include "state.h"
#include <hardware/adc.h>
#include <pico/time.h>
#include <stdlib.h>

typedef struct {
    int16_t x;
    int16_t y; 
} JoystickReading; 

void init_joystick(void) {
    gpio_init(JOYSTICK_POWER);
    gpio_set_dir(JOYSTICK_POWER, GPIO_OUT);
    gpio_put(JOYSTICK_POWER, 1);
    adc_init();
    adc_gpio_init(JOYSTICK_X_GPIO);
    adc_gpio_init(JOYSTICK_Y_GPIO);
}

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
