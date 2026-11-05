#pragma once

#include <stdint.h>

typedef enum { 
    MODE_TV_IR = 0,
    MODE_AIR_MOUSE = 1
} RemoteMode; 

typedef enum {
    ACTION_TOGGLE_MODE, 
    ACTION_POWER, 
    ACTION_VOLUME_UP,
    ACTION_VOLUME_DOWN,
    ACTION_MENU,
    ACTION_CONFIRM, 
}Action; 