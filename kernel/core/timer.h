#ifndef TIMER_H
#define TIMER_H

#include "../kernel_types.h"

// System tick counter (increments on each PIT tick)
extern volatile uint32_t system_ticks;

// Get the current tick count
uint32_t get_ticks(void);

// Increment tick count (called by timer ISR)
void inc_ticks(void);

// Initialize the programmable interval timer (PIT) to the requested
// frequency (Hz). This programs channel 0 of the PIT (ports 0x43/0x40).
void init_timer(uint32_t frequency);

// Halt the CPU until next interrupt
static inline void halt(void) {
    __asm__ volatile ("hlt");
}

#endif // TIMER_H
