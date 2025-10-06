#include "command.h"
#include "../graphics/graphics.h"
#include "../core/string.h"
#include "../kernel_types.h"
#include "../core/io.h"

// Global state
shell_mode_t current_mode = MODE_NORMAL;

// Simple command implementations
void cmd_help(int argc, char** argv) {
    (void)argc; (void)argv;
    gfx_print("Available commands:\n");
    gfx_print("  help    - Show this help message\n");
    gfx_print("  echo    - Display text\n");
    gfx_print("  clear   - Clear the screen\n");
    gfx_print("  version - Show system version\n");
    gfx_print("  reboot  - Restart the system\n");
}

void cmd_echo(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        gfx_print(argv[i]);
        if (i < argc - 1) gfx_print(" ");
    }
    gfx_print("\n");
}

void cmd_clear(int argc, char** argv) {
    (void)argc; (void)argv;
    gfx_clear_screen();
}

void cmd_cls(int argc, char** argv) {
    cmd_clear(argc, argv);
}

void cmd_version(int argc, char** argv) {
    (void)argc; (void)argv;
    gfx_print("QuantumOS v1.0.0-alpha\n");
    gfx_print("Built with keyboard support\n");
}

void cmd_reboot(int argc, char** argv) {
    (void)argc; (void)argv;
    gfx_print("Rebooting system...\n");
    // Simple reboot via keyboard controller
    while ((inb(0x64) & 0x02) != 0);
    outb(0x64, 0xFE);
    __asm__ volatile("hlt");
}

void cmd_shutdown(int argc, char** argv) {
    (void)argc; (void)argv;
    gfx_print("Shutting down...\n");
    // QEMU shutdown
    outw(0x604, 0x2000);
    __asm__ volatile("hlt");
}

void cmd_exit(int argc, char** argv) {
    (void)argc; (void)argv;
    gfx_print("Exit not implemented in kernel mode\n");
}

// Simple command table
typedef struct {
    const char* name;
    void (*func)(int argc, char** argv);
} simple_command_t;

static const simple_command_t commands[] = {
    {"help", cmd_help},
    {"echo", cmd_echo},
    {"clear", cmd_clear},
    {"cls", cmd_cls},
    {"version", cmd_version},
    {"reboot", cmd_reboot},
    {"shutdown", cmd_shutdown},
    {"exit", cmd_exit},
    {NULL, NULL}
};

// Initialize command system
bool command_init(void) {
    gfx_print("Command system initialized\n");
    return true;
}

// Parse input into argc/argv
int parse_input(char* input, char* argv[], int max_args) {
    int argc = 0;
    char* token = input;
    
    // Simple whitespace parsing
    while (*token && argc < max_args - 1) {
        // Skip whitespace
        while (*token == ' ' || *token == '\t') token++;
        
        if (*token == '\0') break;
        
        argv[argc++] = token;
        
        // Find end of token
        while (*token && *token != ' ' && *token != '\t') token++;
        
        if (*token) {
            *token = '\0';
            token++;
        }
    }
    
    argv[argc] = NULL;
    return argc;
}

// Execute command
command_result_t execute_command(const char* input) {
    if (!input || strlen(input) == 0) {
        return CMD_SUCCESS;
    }
    
    // Copy input to avoid modifying original
    char input_copy[256];
    strcpy(input_copy, input);
    
    // Parse into argc/argv
    char* argv[16];
    int argc = parse_input(input_copy, argv, 16);
    
    if (argc == 0) {
        return CMD_SUCCESS;
    }
    
    // Find and execute command
    for (int i = 0; commands[i].name != NULL; i++) {
        if (strcmp(argv[0], commands[i].name) == 0) {
            commands[i].func(argc, argv);
            return CMD_SUCCESS;
        }
    }
    
    gfx_print("Unknown command: ");
    gfx_print(argv[0]);
    gfx_print("\n");
    return CMD_ERROR_UNKNOWN_COMMAND;
}

// Utility functions
bool is_valid_command(const char* name) {
    for (int i = 0; commands[i].name != NULL; i++) {
        if (strcmp(name, commands[i].name) == 0) {
            return true;
        }
    }
    return false;
}

bool check_for_command(const char* cmd) {
    return is_valid_command(cmd);
}

// Stub functions for compatibility
shell_mode_t get_current_mode(void) { return current_mode; }
void set_current_mode(shell_mode_t mode) { current_mode = mode; }
const char* get_mode_string(shell_mode_t mode) { (void)mode; return "normal"; }
void* alloc_temp_buffer(void) { return NULL; }
void release_temp_buffer(void* buffer) { (void)buffer; }
void cmd_bufstatus(int argc, char** argv) { (void)argc; (void)argv; }
// atoi function is provided by string.c