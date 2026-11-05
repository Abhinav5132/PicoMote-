#pragma once
#include <stdbool.h>

void initialize_all_buttons(void);
void poll_joystick(void);
void buttons_enable_dormant_wake(bool enable);
void calibrate_joystick(int samples); 
bool is_joystick_neutral(); 
void process_button_events(void); 