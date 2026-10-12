#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

static uint ir_pwm_slice; 
static uint ir_pwm_channel;

void init_ir(uint pin){
    gpio_set_function(pin, GPIO_FUNC_PWM);
    ir_pwm_slice = pwm_gpio_to_slice_num(pin);
    ir_pwm_channel = pwm_gpio_to_channel(pin);

    uint32_t sys_clk = clock_get_hz(clk_sys);
    uint32_t wrap = (sys_clk/38000) - 1;

    pwm_set_wrap(ir_pwm_slice, wrap);
    pwm_set_chan_level(ir_pwm_slice, ir_pwm_channel, wrap / 3);

    pwm_set_enabled(ir_pwm_slice, false);
}

static inline void ir_mark(uint32_t duration_us){
    pwm_set_enabled(ir_pwm_slice, true);
    sleep_us(duration_us);
}

static inline void ir_space(uint32_t duration_us){
    pwm_set_enabled(ir_pwm_slice, false);
    sleep_us(duration_us);
}

void send_nec(uint8_t address, uint8_t command) {
    uint32_t data = (uint32_t)address | ((uint32_t)(uint8_t)~address << 8) 
        | ((uint32_t)command << 16) | ((uint32_t)(uint8_t)~command <<24);

    ir_mark(9000);
    ir_space(4500);

    for (int i = 0; i<32; i++) {
        ir_mark(560);
        if (data & (1u << i)){
            ir_space(1690);
        } else {
            ir_space(560);
        }
    }

    ir_mark(560);
    ir_space(40000);
}