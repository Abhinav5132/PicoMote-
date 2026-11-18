#include "debug_log.h"
#include "pico/stdio_usb.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#define LOG_CAPACITY 64
static char lines[LOG_CAPACITY][144];
static unsigned head, count;
static uint32_t dropped;

void debug_log(const char *format, ...) {
    if (count == LOG_CAPACITY) {
        ++dropped;
        return;
    }
    unsigned slot = (head + count) % LOG_CAPACITY;
    va_list args;
    va_start(args, format);
    vsnprintf(lines[slot], sizeof(lines[slot]), format, args);
    va_end(args);
    ++count;
}

void debug_log_poll(void) {
    if (!count || !stdio_usb_connected()) return;
    fputs(lines[head], stdout);
    head = (head + 1) % LOG_CAPACITY;
    --count;
    if (dropped) {
        debug_log("[WARN] Dropped %lu log records while buffer was full\n", (unsigned long)dropped);
        dropped = 0;
    }
}
