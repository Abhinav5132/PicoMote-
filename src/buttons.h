#pragma once
#include <stdbool.h>

void initialize_all_buttons(void);
void poll_joystick(void);
void buttons_enable_dormant_wake(bool enable);
