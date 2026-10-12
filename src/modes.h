#pragma once

#include <stdint.h>

typedef enum { 
    MODE_TV_IR = 0,
    MODE_AIR_MOUSE = 1
} RemoteMode; 

typedef enum {
    ACT_IR, 
    ACT_CONTROLLER, 
    ACT_TOGGLE_MODE
} ActionType; 

typedef struct {
    ActionType type; 
    uint32_t val; 
} ButtonBehaviour; 
