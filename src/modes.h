#pragma once

#include <stdint.h>

typedef enum { 
    MODE_TV_IR = 0,
    MODE_AIR_MOUSE = 1
} RemoteMode; 

typedef enum {
    ACTION_UNBOUND = 0, 
    ACTION_TOGGLE_MODE, 
    ACTION_POWER, 
    ACTION_VOLUME_UP,
    ACTION_VOLUME_DOWN,
    ACTION_MENU,
    ACTION_CONFIRM, 
}Action; 

static const char *action_name(Action action) {
    switch (action) {
        case ACTION_UNBOUND: return "UNBOUND";
        case ACTION_TOGGLE_MODE: return "TOGGLE_MODE";
        case ACTION_POWER: return "POWER";
        case ACTION_VOLUME_UP: return "VOLUME_UP";
        case ACTION_VOLUME_DOWN: return "VOLUME_DOWN";
        case ACTION_MENU: return "MENU";
        case ACTION_CONFIRM: return "CONFIRM";
    }
    return "UNKNOWN";
}

static const char *mode_name(RemoteMode mode) {
    switch (mode) {
        case MODE_TV_IR: return "TV_IR";
        case MODE_AIR_MOUSE: return "AIR_MOUSE";
    }
    return "UNKNOWN";
}