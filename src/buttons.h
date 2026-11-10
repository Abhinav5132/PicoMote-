#pragma once
#include <stdbool.h>

void initialize_all_buttons(void);
void buttons_enable_dormant_wake(bool enable);
 
void process_button_events(void); 
bool is_button_queue_empty(void); 