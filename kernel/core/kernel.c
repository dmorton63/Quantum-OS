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
// Function declarations from other modules
void gdt_init(void);
void idt_init(void);  
void interrupts_system_init(void);
void multiboot_parse_info(uint32_t magic, multiboot_info_t* mbi);

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
void kernel_main_loop(void) {
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

/**
 * Main kernel entry point - simplified for keyboard testing
 */
void kernel_main(uint32_t magic, multiboot_info_t* mbi) {
    // Parse multiboot info first to set verbosity level
    multiboot_parse_info(magic, mbi);
    
    // Initialize basic graphics firstma
    graphics_init(mbi);
    framebuffer_init();
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
    idt_init();
    interrupts_system_init();
    
    // Initialize keyboard driver
    gfx_print("Initializing keyboard driver...\n");
    keyboard_init();
    
    gfx_print("QuantumOS kernel initialization complete!\n");
    
    // Enter main kernel loop
    kernel_main_loop();
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
    
    // Halt the system
    while (1) {
        __asm__ volatile("hlt");
    }
}