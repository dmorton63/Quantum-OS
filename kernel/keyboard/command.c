#include "command.h"
#include "../graphics/graphics.h"
#include "../core/string.h"
#include "../core/core_manager.h"
#include "../core/memory/memory_pool.h"
#include "../kernel_types.h"
#include "../core/io.h"
#include "../keyboard/keyboard.h"
//#include "../drivers/usb/usb_mouse.h"
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
    gfx_print("  cores   - Show CPU core allocation map\n");
    gfx_print("  mempool - Show memory pool statistics\n");
    gfx_print("  splash  - Display splash screen from CD-ROM\n");
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

void cmd_kbd(int argc, char** argv) {
    if (argc < 2) {
        gfx_print("Usage: kbd enable|disable|status\n");
        return;
    }

    if (strcmp(argv[1], "enable") == 0) {
        keyboard_set_enabled(true);
        gfx_print("Keyboard processing enabled\n");
    } else if (strcmp(argv[1], "disable") == 0) {
        keyboard_set_enabled(false);
        gfx_print("Keyboard processing disabled\n");
    } else if (strcmp(argv[1], "status") == 0) {
        gfx_print("Keyboard processing is ");
        gfx_print(keyboard_is_enabled() ? "ENABLED\n" : "DISABLED\n");
    } else {
        gfx_print("Unknown kbd command\n");
    }
}

// Forward declaration for PCI scanner
void pci_scan_and_print(void);

void cmd_pci(int argc, char** argv) {
    (void)argc; (void)argv;
    pci_scan_and_print();
}

// External functions for splash screen
extern int iso9660_read_file(const char* path, void* buffer, size_t size, size_t offset);
extern void png_decode_to_framebuffer(const uint8_t* png_data, uint32_t data_size,
                                      uint32_t* framebuffer, uint32_t fb_width, uint32_t fb_height);
extern uint32_t* video_subsystem_get_framebuffer(void);
extern void video_subsystem_get_resolution(uint32_t* width, uint32_t* height);

void cmd_cores(int argc, char** argv) {
    (void)argc; (void)argv;
    
    extern void core_manager_print_allocation_map(void);
    extern core_manager_stats_t* core_manager_get_stats(void);
    
    // Print allocation map
    core_manager_print_allocation_map();
    
    // Print statistics
    core_manager_stats_t* stats = core_manager_get_stats();
    gfx_print("\n=== Core Manager Statistics ===\n");
    gfx_print("Total cores: ");
    gfx_print_hex(stats->total_cores);
    gfx_print("\nAvailable cores: ");
    gfx_print_hex(stats->available_cores);
    gfx_print("\nReserved cores: ");
    gfx_print_hex(stats->reserved_cores);
    gfx_print("\nAllocated cores: ");
    gfx_print_hex(stats->allocated_cores);
    gfx_print("\n");
}

void cmd_mempool(int argc, char** argv) {
    (void)argc; (void)argv;
    
    gfx_print("\n=== Memory Pool Manager Status ===\n\n");
    
    // Display all subsystem memory stats
    extern void memory_pool_print_all_stats(void);
    memory_pool_print_all_stats();
}

void cmd_vmm(int argc, char** argv) {
    (void)argc; (void)argv;
    
    extern uint32_t vmm_alloc_region(uint32_t size);
    extern void vmm_free_region(uint32_t virtual_addr, uint32_t size);
    
    gfx_print("=== Testing Virtual Memory Manager ===\n");
    
    // Test small allocation
    gfx_print("Allocating 4KB region...\n");
    uint32_t region1 = vmm_alloc_region(4096);
    if (region1) {
        gfx_print("Success! Virtual address: ");
        gfx_print_hex(region1);
        gfx_print("\n");
        
        // Try to write to it
        uint32_t* ptr = (uint32_t*)region1;
        *ptr = 0xDEADBEEF;
        
        gfx_print("Wrote 0xDEADBEEF, read back: ");
        gfx_print_hex(*ptr);
        gfx_print("\n");
        
        vmm_free_region(region1, 4096);
        gfx_print("Region freed\n");
    } else {
        gfx_print("Failed to allocate region\n");
    }
    
    gfx_print("\nVMM test complete\n");
}

void cmd_splash(int argc, char** argv) {
    (void)argc; (void)argv;
    
    gfx_print("Loading splash screen from CD-ROM...\n");
    
    // Get framebuffer
    uint32_t* fb = video_subsystem_get_framebuffer();
    uint32_t fb_width, fb_height;
    video_subsystem_get_resolution(&fb_width, &fb_height);
    
    if (!fb) {
        gfx_print("Error: Framebuffer not available\n");
        return;
    }
    
    // Clear framebuffer to black
    for (uint32_t i = 0; i < fb_width * fb_height; i++) {
        fb[i] = 0xFF000000;
    }
    
    // Allocate PNG buffer from VIDEO subsystem pool (2MB)
    uint8_t* png_buffer = (uint8_t*)memory_pool_alloc_large(SUBSYSTEM_VIDEO, 2048 * 1024, 0);
    if (!png_buffer) {
        gfx_print("Failed to allocate PNG buffer\n");
        return;
    }
    
    // Read splash.png from ISO9660
    int bytes_read = iso9660_read_file("/SPLASH.PNG", png_buffer, 2048 * 1024, 0);
    
    if (bytes_read > 0) {
        gfx_print("PNG loaded, decoding...\n");
        png_decode_to_framebuffer((const uint8_t*)png_buffer, bytes_read, fb, fb_width, fb_height);
        gfx_print("Splash screen displayed! Press any key to continue.\n");
    } else {
        gfx_print("Failed to load splash.png from CD-ROM\n");
    }
    
    // Free PNG buffer
    memory_pool_free(SUBSYSTEM_VIDEO, png_buffer);
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
    {"kbd", cmd_kbd},
    {"mempool", cmd_mempool},
    {"vmm", cmd_vmm},
    {"pci", cmd_pci},
    {"cores", cmd_cores},
    {"splash", cmd_splash},
    // {"mouse", cmd_mouse},
    {NULL, NULL}
};


// void cmd_mouse(int argc, char** argv)
// {
//     (void)argc; (void)argv;
//     show_mouse_info();

// }


// void show_mouse_info(void) {
//     if (!mouse_device) {
//         dispatch_logf("Mouse Device: Not Present");
//         return;
//     }

//     dispatch_logf("Mouse Device: Present");
//     dispatch_logf("Vendor ID: 0x%04X", mouse_device->vendor_id);
//     dispatch_logf("Product ID: 0x%04X", mouse_device->product_id);
//     dispatch_logf("Endpoint: 0x%02X (Interrupt IN)", mouse_device->endpoint_address);
//     dispatch_logf("Polling Interval: %d ms", mouse_device->poll_interval);
//     dispatch_logf("Max Packet Size: %d bytes", mouse_device->max_packet_size);

//     if (mouse_device->last_report_len > 0) {
//         dispatch_logf("Last Report: ");
//         for (int i = 0; i < mouse_device->last_report_len; i++) {
//             dispatch_logf(" 0x%02X", mouse_device->last_report[i]);
//         }
//     } else {
//         dispatch_logf("Last Report: None");
//     }
// }
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