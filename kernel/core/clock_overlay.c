#include "clock_overlay.h"
#include "../graphics/framebuffer.h"
#include "../graphics/graphics.h"
#include "../config.h"
#include "../core/sleep.h"
#include "../core/timer.h"


static uint32_t elapsed_seconds = 0;
static bool clock_initialized = false;
static bool clock_visible = true;

static const uint32_t clock_x = 700;
static const uint32_t clock_y = 10;
static const uint32_t clock_width = 80;
static const uint32_t clock_height = 20;

//static const rgb_color_t clock_bg = {0, 0, 0, 128};
static const uint32_t clock_bg_color = 0xFF008000; 

static const rgb_color_t clock_fg = {192, 255, 192, 255};  // pale green-white
static const rgb_color_t clock_bg = {0, 0, 0, 0};  // pale green-white
// static const uint32_t clock_width = 120;
// static const uint32_t clock_height = 32;
uint32_t ticks = 0;
uint32_t shadow_color = 0x202020;

static void draw_clock_box(void) {
    // Shadow layer (optional depth)
    //uint32_t fill = 0x101010;
    uint32_t border = 0x404040;
    //uint32_t shadow_alpha = (ticks % 2 == 0) ? 0x40 : 0x80;
    fb_draw_rect(clock_x + 3, clock_y + 3, clock_width, clock_height,
                 clock_bg_color);  // translucent black (ARGB)

    // Main fill
fb_draw_rect(clock_x + 4, clock_y + 4, clock_width, clock_height, shadow_color);  // shadow
fb_draw_rect(clock_x, clock_y, clock_width, clock_height, clock_bg_color);         // main box
fb_draw_rect_outline(clock_x, clock_y, clock_width, clock_height, border); // border // white border
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

    fb_draw_text_with_bg(clock_x + 10, clock_y + 6, time_str, clock_fg, clock_bg);
}

void clock_tick(void) {
    static uint32_t last_ticks = 0;
    ticks = get_ticks();
    if ((ticks - last_ticks) >= 100) {  // 100 ticks = 1 second at 100Hz
        last_ticks = ticks;
        elapsed_seconds++;
        draw_clock();
    }

}

void reset_clock(void) {
    elapsed_seconds = 0;
    if (clock_initialized) draw_clock();
   // clock_loop();
}

void clock_overlay_init(void) {
    clock_initialized = true;
    reset_clock();
}

void clock_loop() {
    while (1) {
        clock_tick();
        sleep_ms(500);
    }
}

