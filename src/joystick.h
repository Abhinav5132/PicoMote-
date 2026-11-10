#pragma once
#include <stdbool.h>
void calibrate_joystick(int samples); 
bool is_joystick_neutral();
void poll_joystick(void);
void initialize_joystick(void);
