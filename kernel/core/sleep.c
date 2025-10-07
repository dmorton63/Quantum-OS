#include "sleep.h"
#include "timer.h"

// Simple millisecond sleep using system tick counter
void sleep_ms(uint32_t ms) {
    uint32_t target_ticks = get_ticks() + (ms / MS_PER_TICK);
    while (get_ticks() < target_ticks) {
        halt();  // HLT wrapper from timer.h
    }
}