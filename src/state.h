#pragma once
#include "modes.h"
#include <stdbool.h>

typedef enum {
    SLEEP,
    AWAKE
} RemoteSleepState; 

RemoteMode get_mode(void);
void toggle_mode(void);
void set_mode(RemoteMode mode);
void sleep(void);
void wake(void);
RemoteSleepState get_sleep_state(void);
bool should_sleep(uint32_t timeout_ms);
uint32_t get_last_active_time(); 