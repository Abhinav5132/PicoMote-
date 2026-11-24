#pragma once

// Main-loop only. Interrupt handlers pass timestamps through the event queue.
void debug_log(const char *format, ...) __attribute__((format(printf, 1, 2)));
void debug_log_poll(void);
