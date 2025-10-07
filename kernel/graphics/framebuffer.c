/**
 * QuantumOS - Framebuffer Graphics Backend
 * Linear framebuffer support for VESA/GOP modes
 */
#include "framebuffer.h"
#include "../graphics/graphics.h"
#include "font_data.h"
#include "../core/io.h"
#include "../core/boot_log.h"
#include "../config.h"

// Debug functions now handled by config.h macros

// Forward declarations
void framebuffer_scroll(void);
void framebuffer_draw_char(uint32_t x, uint32_t y, char c, rgb_color_t fg, rgb_color_t bg);

static uint32_t* framebuffer_ptr = NULL;
static uint32_t fb_width = 0;
static uint32_t fb_height = 0;
static uint32_t fb_pitch = 0;
static uint32_t fb_bpp = 0;
static uint32_t cursor_x = 0;
static uint32_t cursor_y = 0;

// Use vid_layer font system (8x8 fonts)
// External font functions from fonts.c
extern font_descriptor_t* get_font(font_type_t type);
extern void draw_bitmap_char(uint32_t fb_x, uint32_t fb_y, char c, rgb_color_t fg_color, rgb_color_t bg_color, font_descriptor_t* font);

// Use vid_layer 8x8 fonts instead of basic_font
#define FONT_WIDTH 8
#define FONT_HEIGHT 8

void framebuffer_init(void) {
    display_info_t* info = graphics_get_display_info();
    
    // Get framebuffer info from multiboot (stored during init)
    // The graphics system should have preserved the real framebuffer info
    if (info->framebuffer != NULL) {
        framebuffer_ptr = info->framebuffer;
        
        // Store the PIXEL dimensions from multiboot detection
        // Note: At this point info->width/height are still in PIXELS from multiboot
        uint32_t pixel_width = info->width;
        uint32_t pixel_height = info->height;
        
        // Use actual framebuffer parameters from multiboot
        fb_width = pixel_width;
        fb_height = pixel_height;  
        fb_bpp = info->bpp;
        fb_pitch = info->pitch;
        
        // Boot log framebuffer detection
        BOOT_LOG("Framebuffer detected and configured\n");
        BOOT_LOG_HEX("FB Address: ", (uint32_t)framebuffer_ptr);
        BOOT_LOG_DEC("FB Resolution: ", fb_width);
        BOOT_LOG_DEC("x", fb_height);
        BOOT_LOG_DEC(" BPP: ", fb_bpp);
        BOOT_LOG_DEC(" Pitch: ", fb_pitch);
        SERIAL_LOG_MIN("FB_INIT: Framebuffer available\n");
    } else {
        // No framebuffer available
        framebuffer_ptr = NULL;
        
        SERIAL_LOG_MIN("FB_INIT: No framebuffer available!\n");
        return;
    }
    
    cursor_x = 0;
    cursor_y = 0;
    
    // Update display info for TEXT operations (characters, not pixels)
    info->width = fb_width / FONT_WIDTH;    // Characters per line
    info->height = fb_height / FONT_HEIGHT; // Lines per screen
    info->pitch = fb_pitch;
    info->bpp = fb_bpp;
    
    // Debug: Show text area dimensions
    BOOT_LOG_DEC("Text cols: ", info->width);
    BOOT_LOG_DEC("Text rows: ", info->height);
    // Keep framebuffer pointer for graphics operations
    info->cursor_x = 0;
    info->cursor_y = 0;
    
    // Don't auto-clear the framebuffer - let existing content show
    // framebuffer_clear(); // Commented out to preserve initialization messages
    
    // DEBUG: Force draw some test characters directly to verify framebuffer works
    framebuffer_draw_char(0, 0, 'T', (rgb_color_t){255,255,255,255}, (rgb_color_t){0,0,0,255});
    framebuffer_draw_char(8, 0, 'E', (rgb_color_t){255,255,255,255}, (rgb_color_t){0,0,0,255});
    framebuffer_draw_char(16, 0, 'S', (rgb_color_t){255,255,255,255}, (rgb_color_t){0,0,0,255});
    framebuffer_draw_char(24, 0, 'T', (rgb_color_t){255,255,255,255}, (rgb_color_t){0,0,0,255});
}

void framebuffer_putchar(char c) {
    display_info_t* info = graphics_get_display_info();
    if (!framebuffer_ptr || !info) return;

    static int call_count = 0;
    if (call_count < 10) {

        call_count++;
    }

    uint32_t max_cols = info->width;   // info->width is already in characters
    uint32_t max_rows = info->height;  // info->height is already in characters

    switch (c) {
        case '\n':
            cursor_x = 0;
            cursor_y++;
            if (cursor_y >= max_rows) {
                framebuffer_scroll();
                cursor_y = max_rows - 1;
            }
            break;

        case '\r':
            cursor_x = 0;
            break;

        case '\b':
            if (cursor_x > 0) {
                cursor_x--;
                rgb_color_t bg = color_to_rgb(info->bg_color);
                framebuffer_draw_char(cursor_x * FONT_WIDTH,
                                      cursor_y * FONT_HEIGHT,
                                      ' ', bg, bg);
            }
            break;

        case '\t':
            cursor_x = (cursor_x + 4) & ~3;
            if (cursor_x >= max_cols) {
                cursor_x = 0;
                cursor_y++;
                if (cursor_y >= max_rows) {
                    framebuffer_scroll();
                    cursor_y = max_rows - 1;
                }
            }
            break;

        default:
            if (c >= 32 && c <= 126) {
                rgb_color_t fg = {255, 255, 255, 255}; // white
                rgb_color_t bg = {0, 0, 0, 255};       // black

                framebuffer_draw_char(cursor_x * FONT_WIDTH,
                                      cursor_y * FONT_HEIGHT,
                                      c, fg, bg);

                cursor_x++;
                if (cursor_x >= max_cols) {
                    cursor_x = 0;
                    cursor_y++;
                    if (cursor_y >= max_rows) {
                        framebuffer_scroll();
                        cursor_y = max_rows - 1;
                    }
                }
            }
            break;
    }

    framebuffer_set_cursor(cursor_x, cursor_y);
    info->cursor_x = cursor_x;
    info->cursor_y = cursor_y;
}

void framebuffer_clear(void) {
    display_info_t* info = graphics_get_display_info();
    
    if (!framebuffer_ptr) return;
    
    rgb_color_t bg = color_to_rgb(info->bg_color);
    uint32_t pixel = rgb_to_pixel(bg, fb_bpp, 16, 8, 0); // R=16, G=8, B=0 for RGBA
    
    // Clear entire framebuffer
    for (uint32_t y = 0; y < fb_height; y++) {
        for (uint32_t x = 0; x < fb_width; x++) {
            uint32_t offset = (y * fb_pitch) / 4 + x;
            framebuffer_ptr[offset] = pixel;
        }
    }
    
    cursor_x = 0;
    cursor_y = 0;
    info->cursor_x = 0;
    info->cursor_y = 0;
}

void splash_clear(rgb_color_t bg) {
    for (uint32_t y = 0; y < fb_height; y++) {
        for (uint32_t x = 0; x < fb_width; x++) {
            framebuffer_draw_pixel(x, y, bg);
        }
    }
}

void splash_box(uint32_t w, uint32_t h, rgb_color_t color) {
    uint32_t x0 = (fb_width - w) / 2;
    uint32_t y0 = (fb_height - h) / 2;

    for (uint32_t y = 0; y < h; y++) {
        for (uint32_t x = 0; x < w; x++) {
            framebuffer_draw_pixel(x0 + x, y0 + y, color);
        }
    }
}

void splash_title(const char* text, rgb_color_t fg, rgb_color_t bg) {
    uint32_t len = strlen(text);
    uint32_t x0 = (fb_width / 2) - (len * 8 / 2);
    uint32_t y0 = fb_height / 2 - 8;

    for (uint32_t i = 0; i < len; i++) {
        framebuffer_draw_char(x0 + i * 8, y0, text[i], fg, bg);
    }
}

void framebuffer_set_cursor(uint32_t x, uint32_t y) {
    display_info_t* info = graphics_get_display_info();

    if (x < info->width && y < info->height) {
        cursor_x = x;
        cursor_y = y;
        info->cursor_x = x;
        info->cursor_y = y;
    }
}

void framebuffer_scroll(void) {
    display_info_t* info = graphics_get_display_info();
    
    if (!framebuffer_ptr) return;
    
    uint32_t line_height = FONT_HEIGHT;
    (void)fb_bpp; // Suppress unused warning
    
    // Move all lines up by one character line
    for (uint32_t y = 0; y < fb_height - line_height; y++) {
        for (uint32_t x = 0; x < fb_width; x++) {
            uint32_t src_offset = ((y + line_height) * fb_pitch) / 4 + x;
            uint32_t dst_offset = (y * fb_pitch) / 4 + x;
            framebuffer_ptr[dst_offset] = framebuffer_ptr[src_offset];
        }
    }
    
    // Clear the last line
    rgb_color_t bg = color_to_rgb(info->bg_color);
    uint32_t pixel = rgb_to_pixel(bg, fb_bpp, 16, 8, 0);
    
    for (uint32_t y = fb_height - line_height; y < fb_height; y++) {
        for (uint32_t x = 0; x < fb_width; x++) {
            uint32_t offset = (y * fb_pitch) / 4 + x;
            framebuffer_ptr[offset] = pixel;
        }
    }
    if (cursor_x * FONT_WIDTH >= fb_width) {
    cursor_x = 0;
    cursor_y++;
    }

    info->cursor_x = cursor_x;
    info->cursor_y = cursor_y;
}

// Framebuffer-specific drawing functions
void framebuffer_draw_pixel(uint32_t x, uint32_t y, rgb_color_t color) {
    if (!framebuffer_ptr || x >= fb_width || y >= fb_height) return;

    // Convert rgb_color_t to packed 32-bit pixel (BGRA format)
    uint32_t pixel = (color.blue << 0) | (color.green << 8) | (color.red << 16) | (color.alpha << 24);

    // Calculate offset in bytes, then divide by 4 to index into uint32_t*
    uint32_t offset = (y * fb_pitch + x * (fb_bpp / 8)) / 4;

    framebuffer_ptr[offset] = pixel;
}

extern const uint8_t vga_font[128][8];

void framebuffer_draw_char(uint32_t x, uint32_t y, char c, rgb_color_t fg, rgb_color_t bg) {
    if (!framebuffer_ptr || x >= fb_width || y >= fb_height) return;
    const uint8_t* glyph = vga_font[(uint8_t)c];
    uint32_t fg_pixel = (fg.blue << 0) | (fg.green << 8) | (fg.red << 16) | (fg.alpha << 24);
    uint32_t bg_pixel = (bg.blue << 0) | (bg.green << 8) | (bg.red << 16) | (bg.alpha << 24);

    for (uint32_t row = 0; row < FONT_HEIGHT; row++) {
        uint8_t bits = glyph[row];
        //SERIAL_LOG_HEX(" 0x", glyph[row]);
        for (uint32_t col = 0; col < FONT_WIDTH; col++) {
            uint32_t pixel_x = x + col;
            uint32_t pixel_y = y + row;
            if (pixel_x < fb_width && pixel_y < fb_height) {
                uint32_t offset = (pixel_y * fb_pitch + pixel_x * (fb_bpp / 8)) / 4;
                framebuffer_ptr[offset] = (bits & (1 << col)) ? fg_pixel : bg_pixel;
            }
        }
    }
}



// VESA/GOP detection and initialization (placeholder)
bool framebuffer_detect_vesa(void) {
    // TODO: Implement VESA mode detection
    // This would involve calling VESA BIOS functions
    return false;
}

bool framebuffer_detect_gop(void) {
    // TODO: Implement UEFI GOP detection
    // This would involve UEFI protocol calls
    return false;
}

void framebuffer_set_mode(uint32_t width, uint32_t height, uint32_t bpp) {
    // TODO: Implement mode setting via VESA/GOP
    fb_width = width;
    fb_height = height;
    fb_bpp = bpp;
    fb_pitch = width * (bpp / 8);
    
    display_info_t* info = graphics_get_display_info();
    info->width = width / FONT_WIDTH;
    info->height = height / FONT_HEIGHT;
    info->pitch = fb_pitch;
    info->bpp = bpp;
}

// Test function to verify framebuffer is accessible
void framebuffer_test(void) {
    if (!framebuffer_ptr) {
        SERIAL_LOG("FB_TEST: No framebuffer available!\n");
        return;
    }
    

    // Draw some test pixels to verify framebuffer works
    rgb_color_t red = {0xFF, 0x00, 0x00, 0xFF};
    rgb_color_t green = {0x00, 0xFF, 0x00, 0xFF};
    rgb_color_t blue = {0x00, 0x00, 0xFF, 0xFF};
    for (uint32_t y = 0; y < 100; y++) {
        for (uint32_t x = 0; x < 100; x++) {
            framebuffer_draw_pixel(x, y, red); // Magenta
        }
    }    
        // Draw a few colored pixels in top-left corner
    framebuffer_draw_pixel(0, 0, red);
    framebuffer_draw_pixel(1, 0, green);
    framebuffer_draw_pixel(2, 0, blue);
    framebuffer_draw_pixel(0, 1, green);
    framebuffer_draw_pixel(1, 1, blue);
    framebuffer_draw_pixel(2, 1, red);
}