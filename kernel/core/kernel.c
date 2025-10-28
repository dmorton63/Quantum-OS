/**
 * QuantumOS Kernel - Main Entry Point (Simplified for Keyboard Testing)
 * 
 * This is the main kernel entry point focused on getting keyboard input working.
 */

#include "multiboot.h"
#include "string.h"
#include "../config.h"
#include "../keyboard/keyboard.h"
#include "../shell/shell.h"
#include "../graphics/graphics.h"
#include "../graphics/framebuffer.h"
#include "kernel.h"
#include "../graphics/popup.h"
#include "../graphics/message_box.h"
#include "../qarma_win_handle/qarma_win_handle.h"
#include "../qarma_win_handle/qarma_window_manager.h"
#include "../qarma_win_handle/panic.h"

//#include "../scheduler/qarma_win_handle.h"
//#include "../scheduler/qarma_splash_app.h"  // Contains splash_app and its functions

extern QARMA_WIN_HANDLE global_win_handler;
extern QARMA_APP_HANDLE splash_app;


// Function declarations from other modules
void gdt_init(void);
void idt_init(void);  
void interrupts_system_init(void);
void multiboot_parse_info(uint32_t magic, multiboot_info_t* mbi);
uint32_t get_ticks(void);

// Basic types
typedef unsigned int size_t;

// Global verbosity level  
verbosity_level_t g_verbosity = VERBOSITY_VERBOSE;

// I/O port functions (temporary - should move to a dedicated header)
uint8_t inb(uint16_t port) {
    uint8_t result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

void outb(uint16_t port, uint8_t data) {
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

// Serial debug functions for keyboard and other subsystems
void serial_debug(const char* msg) {
    const char* ptr = msg;
    while (*ptr) {
        // Write to COM1 (0x3F8)
        while ((inb(0x3F8 + 5) & 0x20) == 0);
        outb(0x3F8, *ptr);
        ptr++;
    }
}

void serial_debug_hex(uint32_t value) {
    char hex_chars[] = "0123456789ABCDEF";
    char buffer[9] = "00000000";
    for (int i = 7; i >= 0; i--) {
        buffer[7-i] = hex_chars[(value >> (i * 4)) & 0xF];
    }
    serial_debug(buffer);
}

void serial_debug_decimal(uint32_t value) {
    char buffer[11]; // Max for 32-bit: 4294967295 + null terminator
    char* ptr = buffer + 10;
    *ptr = '\0';
    
    do {
        *--ptr = '0' + (value % 10);
        value /= 10;
    } while (value > 0);
    
    serial_debug(ptr);
}

/**
 * Main kernel loop for keyboard testing
 */
int kernel_main_loop(void) {
    gfx_print("\n=== QuantumOS Interactive Shell ===\n");
    gfx_print("Keyboard input enabled. Type 'help' for commands.\n\n");
    
    // Initialize shell interface
    shell_init();
    
    // Enable interrupts for keyboard input
    __asm__ volatile("sti");
    
    // Interactive loop
    while (1) {
        // CPU halt to reduce CPU usage while waiting for input
        __asm__ volatile("hlt");
    }
}


int kernel_splash_test()
{
    // Initialize splash app
    splash_app.init(&splash_app);

    uint32_t last_tick = get_ticks();
    QARMA_TICK_CONTEXT ctx = {
        .tick_count = 0,
        .delta_time = 0.0f,
        .uptime_seconds = 0.0f
    };

    while (true) {
        uint32_t current_tick = get_ticks();
        if (current_tick > last_tick) {
            uint32_t ticks_elapsed = current_tick - last_tick;
            last_tick = current_tick;

            ctx.tick_count += ticks_elapsed;
            ctx.delta_time = (float)ticks_elapsed / (float)QARMA_TICK_RATE;
            ctx.uptime_seconds += ctx.delta_time;

            // Update splash app
            splash_app.update(&splash_app, &ctx);

            // Update all windows via manager
            qarma_window_manager.update_all(&qarma_window_manager, &ctx);

            // Render all windows
            qarma_window_manager.render_all(&qarma_window_manager);

            // Exit when splash window is gone
            if (splash_app.main_window == NULL) {
                break;
            }
        }

        sleep_ms(1);  // Let interrupts breathe
    }
    
    splash_app.shutdown(&splash_app);
    return 0;
}

/**
 * Main kernel entry point - simplified for keyboard testing
 */
int kernel_main(uint32_t magic, multiboot_info_t* mbi) {
    // Parse multiboot info first to set verbosity level
    multiboot_parse_info(magic, mbi);
    
    // Initialize basic graphics firstma
    graphics_init(mbi);
    framebuffer_init();
     /* Initialize message box as soon as framebuffer is available so
         any early debug messages are captured and displayed in the UI. */
    //  message_box_init(80);
    //  message_box_push("Framebuffer initialized.");
    // framebuffer_test();
    //     while (1) {
    //     // CPU halt to reduce CPU usage while waiting for input
    //     __asm__ volatile("hlt");
    // }
    gfx_print("=== QuantumOS v1.0 Starting ===\n");
    gfx_print("Keyboard Testing Version\n");
    
    // Initialize GDT
    gfx_print("Initializing GDT...\n");
    gdt_init();
    
    // Initialize IDT and interrupts
    gfx_print("Initializing IDT and interrupts...\n");
    //idt_init();
    __asm__ volatile("cli");
    interrupts_system_init();
    
    // Initialize keyboard driver
    gfx_print("Initializing keyboard driver...\n");
    //draw_splash("QuantumOS Keyboard Test");
    keyboard_init();
    // Ensure higher-level keyboard processing is enabled by default so
    // the interactive shell and commands are available. Use the `kbd`
    // command at runtime to toggle processing if needed.
    keyboard_set_enabled(true);
    // now that keyboard and IDT are initialized so
    // modal popups can use the interrupt-driven keyboard buffer.
    __asm__ volatile("sti");
    //message_box_push("System initialized. Ready."); 
    gfx_print("Keyboard driver initialized.\n");
    
    //__asm__ volatile("int $0x2c");
    rgb_color_t deep_blue = { .red = 0x00, .green = 0x33, .blue = 0x66 };
    rgb_color_t splash_bg = { .red = 0x46, .green = 0x82, .blue = 0xB4, .alpha = 0xFF }; // Steel Blue
    rgb_color_t text_fg = { .red = 0x00, .green = 0x00, .blue = 0x00, .alpha = 0xFF }; // White
    splash_clear(deep_blue);

    splash_clear(deep_blue);
  
    splash_box(400, 200, splash_bg);
    splash_title("QARMA with Keyboard Test", text_fg, splash_bg);

    //kernel_splash_test();
     /* Redraw the message box after splash is drawn since splash_clear()
         overwrites the framebuffer. This ensures the bottom message area is
         visible to the user. */
     //extern void message_box_render(void);
    //  message_box_render();
    //shell_run();
    
//     PopupParams params = {
//     .x = 100,
//     .y = 80,
//     .message = "Save changes?",
//     .title = "Confirm",
//     .title_color = { .red = 0xFF, .green = 0xFF, .blue = 0xFF, .alpha = 0xFF },
//     .title_height = 26,
//     .bg_color = (rgb_color_t){ .red = 0x22, .green = 0x22, .blue = 0x22, .alpha = 0xFF },
//     .border_color = (rgb_color_t){ .red = 0xFF, .green = 0xFF, .blue = 0xFF, .alpha = 0xFF },
//     .text_color = { .red = 0x00, .green = 0xFF, .blue = 0x00, .alpha = 0xFF },  // if using rgb_color_t
//     .timeout_ms = 0,
//     .dismiss_on_esc = true,
//     .confirm_on_enter = true,
//     .on_confirm = confirm_action,
//     .on_cancel = cancel_action
// };



// show_popup(&params);
    //gfx_print("QuantumOS kernel initialization complete!\n");
     //   __asm__ volatile("cli");
        __asm__ volatile("sti");
    // Enter main kernel loop
    //kernel_main_loop();
    // while(1)
    // {

    //     __asm__ volatile("hlt");
    // }

    return 1;
}

/**
 * Early kernel initialization - called by boot stub
 */
void kernel_early_init(void) {
    // Initialize critical systems first
    gdt_init();
    
    // Basic graphics initialization for early logging
    //graphics_init();
    
    gfx_print("Early kernel initialization complete.\n");
}

/**
 * Handle critical kernel panics
 */
void kernel_panic(const char* message) {
    // Disable interrupts
    __asm__ volatile("cli");
    
    gfx_print("\n*** KERNEL PANIC ***\n");
    gfx_print("Error: ");
    gfx_print(message);
    gfx_print("\nSystem halted.\n");
    
    // // Halt the system
    // while (1) {
    //     __asm__ volatile("hlt");
    // }
    panic(message);
}



const char* splash[] = {
    "╔══════════════════════════════════════╗",
    "║         Welcome to QuantumOS        ║",
    "║        The Ritual Has Begun         ║",
    "╚══════════════════════════════════════╝",
};

void draw_splash(const char* title) {
    volatile uint16_t* fb = (uint16_t*)0xB8000;
    uint8_t attr = (BLUE << 4) | WHITE;

    for (int i = 0; i < 80 * 25; i++) {
        fb[i] = (attr << 8) | ' ';
    }

    for (int i = 0; title[i]; i++) {
        fb[40 - (strlen(title) / 2) + i] = (attr << 8) | title[i];
    }
}


