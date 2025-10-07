#include "timer.h"
#include "../core/io.h"

#define PIT_COMMAND_PORT 0x43
#define PIT_CHANNEL0_PORT 0x40

// PIT input frequency is 1193182 Hz
#define PIT_BASE_FREQUENCY 1193182u

volatile uint32_t system_ticks = 0;

uint32_t get_ticks(void) {
    return system_ticks;
}

void inc_ticks(void) {
    system_ticks++;
}

void init_timer(uint32_t frequency) {
    if (frequency == 0) return;
    uint32_t divisor = PIT_BASE_FREQUENCY / frequency;
    uint8_t low = divisor & 0xFF;
    uint8_t high = (divisor >> 8) & 0xFF;

    // Command byte: channel 0, lobyte/hibyte, mode 2 (rate generator), binary
    outb(PIT_COMMAND_PORT, 0x34);
    outb(PIT_CHANNEL0_PORT, low);
    outb(PIT_CHANNEL0_PORT, high);
}
