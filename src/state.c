#include "state.h"

static RemoteMode current_mode = MODE_TV_IR;

RemoteMode get_mode(void) {
    return current_mode;
}

void toggle_mode(void) {
    current_mode = (current_mode == MODE_TV_IR) ? MODE_AIR_MOUSE : MODE_TV_IR;
}

void set_mode(RemoteMode mode) {
    current_mode = mode;
}
