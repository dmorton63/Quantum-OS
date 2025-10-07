#pragma once

#define COLOR_CLOCK_TEXT 0x00FF00  // Bright green
#define COLOR_CLOCK_BG 0x000000    // Black background

void clock_tick(void);
void draw_clock(void);
void reset_clock(void);
void toggle_clock_visibility(void);
void clock_overlay_init(void);
void clock_loop();
// Show/hide clock overlay