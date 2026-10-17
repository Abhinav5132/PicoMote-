#pragma once

#include "modes.h"

void save_state_to_powman(RemoteMode mode); 
RemoteMode restore_state_from_powman(void); 
void enter_dormant_sleep(void); 
void return_from_dormant_sleep(void);
void enter_pstate(void);
void manage_power(void); 
