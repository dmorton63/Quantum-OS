#include "shell.h"
#include "../graphics/graphics.h"
#include "../keyboard/command.h"
#include "../core/string.h"

shell_state_t g_shell_state = { .current_path = "/", .initialized = false };

void shell_init(void) {
    if (g_shell_state.initialized) {
        return;
    }
    
    strcpy(g_shell_state.current_path, "/");
    g_shell_state.initialized = true;
    
    gfx_print("QuantumOS Shell Initialized\n");
    gfx_print("Type 'help' for available commands\n\n");
    show_prompt("/");
}

void show_prompt(const char* path) {
    gfx_print("QuantumOS:");
    gfx_print(path);
    gfx_print("$ ");
}

void process_command(const char* command) {
    if (!command || strlen(command) == 0) {
        return;
    }
    
    // Execute the command using the command system
    execute_command(command);
}

void screen_printf(const char* format, ...) {
    // Simple implementation - just pass to gfx_print for now
    // In a full implementation, this would handle printf-style formatting
    gfx_print(format);
}

// Printf-style function for commands
void gfx_printf(const char* format, ...) {
    // Simple implementation - just pass to gfx_print for now
    gfx_print(format);
}

// Screen manipulation functions are provided by graphics subsystem

void screen_put_char(char c) {
    gfx_putchar(c);
}