#include "clock_overlay.h"
#include "../graphics/framebuffer.h"
#include "../graphics/graphics.h"
#include "../config.h"
#include "../core/sleep.h"

static uint32_t elapsed_seconds = 0;
static bool clock_initialized = false;
static bool clock_visible = true;

static const uint32_t clock_x = 700;
static const uint32_t clock_y = 10;
static const uint32_t clock_width = 80;
static const uint32_t clock_height = 20;

static const rgb_color_t clock_fg = {255, 255, 255, 255};
static const rgb_color_t clock_bg = {0, 0, 0, 128};

static void draw_clock_box(void) {
    for (uint32_t y = 0; y < clock_height; y++) {
        for (uint32_t x = 0; x < clock_width; x++) {
            framebuffer_draw_pixel(clock_x + x, clock_y + y, clock_bg);
        }
    }
}

void format_time(char *buffer, uint32_t seconds) {
    int hrs = seconds / 3600;
    int mins = (seconds % 3600) / 60;
    int secs = seconds % 60;

    buffer[0] = '0' + (hrs / 10);
    buffer[1] = '0' + (hrs % 10);
    buffer[2] = ':';
    buffer[3] = '0' + (mins / 10);
    buffer[4] = '0' + (mins % 10);
    buffer[5] = ':';
    buffer[6] = '0' + (secs / 10);
    buffer[7] = '0' + (secs % 10);
    buffer[8] = '\0';
}

void draw_clock(void) {
    if (!clock_visible) return;
    draw_clock_box();
    char time_str[9];
    format_time(time_str, elapsed_seconds);
    fb_draw_text(clock_x + 10, clock_y + 6, time_str, clock_fg);
}

void clock_tick(void) {
    elapsed_seconds++;
    draw_clock();
}

void reset_clock(void) {
    elapsed_seconds = 0;
    if (clock_initialized) draw_clock();
    clock_loop();
}

void clock_overlay_init(void) {
    clock_initialized = true;
    reset_clock();
}

void clock_loop() {
    while (1) {
        clock_tick();
        sleep_ms(500000);
    }
}

