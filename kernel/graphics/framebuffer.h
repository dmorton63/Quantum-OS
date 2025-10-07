#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "../core/stdtools.h"
#include "graphics.h"

// Framebuffer functions
void framebuffer_init(void);
void framebuffer_putchar(char c);
void framebuffer_clear(void);
void framebuffer_set_cursor(uint32_t x, uint32_t y);
void framebuffer_scroll(void);
void splash_clear(rgb_color_t bg);
void splash_box(uint32_t w, uint32_t h, rgb_color_t color);
void splash_title(const char *text, rgb_color_t fg, rgb_color_t bg);
void framebuffer_draw_pixel(uint32_t x, uint32_t y, rgb_color_t color);
void framebuffer_draw_char(uint32_t x, uint32_t y, char c, rgb_color_t fg, rgb_color_t bg);
void framebuffer_test(void);

void fb_draw_text(uint32_t x, uint32_t y, const char *text, rgb_color_t color);

// VESA/GOP mode detection (placeholders for future implementation)
bool framebuffer_detect_vesa(void);
bool framebuffer_detect_gop(void);
void framebuffer_set_mode(uint32_t width, uint32_t height, uint32_t bpp);

void fb_draw_rect(int x, int y, int width, int height, uint32_t color);
void fb_draw_rect_outline(int x, int y, int width, int height, uint32_t color);
uint32_t fb_get_pixel(int x, int y);

// Raw framebuffer pointer (used by some helpers)
extern uint32_t* fb_ptr;



#endif // FRAMEBUFFER_H