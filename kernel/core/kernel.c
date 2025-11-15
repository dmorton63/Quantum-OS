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
#include "../graphics/subsystem/video_subsystem.h"
#include "../core/scheduler/subsystem_registry.h"
#include "../graphics/framebuffer.h"
#include "kernel.h"
#include "../graphics/popup.h"
#include "../graphics/message_box.h"
#include "../qarma_win_handle/qarma_win_handle.h"
#include "../qarma_win_handle/qarma_window_manager.h"
#include "../qarma_win_handle/panic.h"
#include "../core/memory.h"
#include "../core/memory/memory_pool.h"
#include "../core/input/mouse.h"
#include "../core/pci.h"
#include "../core/scheduler/task_manager.h"
#include "../core/scheduler/scheduler_demo.h"
#include "../fs/file_subsystem/file_subsystem.h"
#include "../fs/vfs.h"
#include "../fs/iso9660.h"
#include "../graphics/png_decoder.h"
#include "../drivers/block/cdrom.h"
#include "../core/blockdev.h"
#include "../core/memory/heap.h"


// #include "../core/memory/vmm/vmm.h"
// #include "../core/memory/heap.h"
// #include "../core/overlay/overlay.h"
//#include "../scheduler/qarma_window_manager.h"

//#include "../scheduler/qarma_win_handle.h"
#include "../splash_app/qarma_splash_app.h"  // Contains splash_app and its functions
#include "../graphics/png_decoder.h"  // For PNG loading functions

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
// int kernel_main_loop(void) {
//     gfx_print("\n=== QuantumOS Interactive Shell ===\n");
//     gfx_print("Keyboard input enabled. Type 'help' for commands.\n\n");
    
//     // Initialize shell interface
//     shell_init();
    
//     // Enable interrupts for keyboard input
//     __asm__ volatile("sti");
    
//     // Interactive loop
//     while (1) {
//         // CPU halt to reduce CPU usage while waiting for input
//         __asm__ volatile("hlt");
//     }
// }


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
    // Early debug output using VGA text mode
    volatile char* vga_buffer = (volatile char*)0xB8000;
    const char* msg = "BOOT: kernel_main started     ";
    for (int i = 0; msg[i] != '\0'; i++) {
        vga_buffer[80*2 + i * 2] = msg[i];      // Second line
        vga_buffer[80*2 + i * 2 + 1] = 0x07;   // White on black
    }

    memory_init();   // Parse multiboot info first to set verbosity level
    
    // Update VGA output
    const char* msg2 = "BOOT: memory_init complete    ";
    for (int i = 0; msg2[i] != '\0'; i++) {
        vga_buffer[160*2 + i * 2] = msg2[i];    // Third line
        vga_buffer[160*2 + i * 2 + 1] = 0x07;
    }
    
    multiboot_parse_info(magic, mbi);
    
    const char* msg3 = "BOOT: multiboot parsed        ";
    for (int i = 0; msg3[i] != '\0'; i++) {
        vga_buffer[240*2 + i * 2] = msg3[i];    // Fourth line
        vga_buffer[240*2 + i * 2 + 1] = 0x07;
    }
    
   // Physical memory manager (frame allocator, memory map)

    // Initialize basic graphics firstma
    const char* msg4 = "BOOT: starting graphics init  ";
    for (int i = 0; msg4[i] != '\0'; i++) {
        vga_buffer[320*2 + i * 2] = msg4[i];    // Fifth line
        vga_buffer[320*2 + i * 2 + 1] = 0x07;
    }
    
    graphics_init(mbi);
    framebuffer_init();
    
    // Initialize subsystem registry
    subsystem_registry_init();
    gfx_print("Subsystem registry initialized.\n");
    
    // Initialize parallel processing engine (needed for core management)
    parallel_engine_init();
    gfx_print("Parallel processing engine initialized.\n");
    
    // Initialize core allocation manager
    extern void core_manager_init(void);
    core_manager_init();
    gfx_print("Core allocation manager initialized.\n");
    
    // Initialize memory pool manager
    extern void memory_pool_init(void);
    memory_pool_init();
    gfx_print("Memory pool manager initialized.\n");
    
    // Initialize video subsystem
    video_subsystem_init(NULL);
    gfx_print("Video subsystem initialized.\n");
    
    // TEST PNG LOADING - moved to early boot
    SERIAL_LOG("===EARLY PNG TEST START===\n");
    gfx_print("===EARLY PNG TEST START===\n");
    png_image_t* early_splash = load_splash_image();
    if (early_splash) {
        SERIAL_LOG("SUCCESS: PNG image loaded and decoded!\n");
        gfx_print("SUCCESS: PNG image loaded and decoded!\n");
        gfx_print("Image dimensions: ");
        // Simple number printing
        uint32_t w = early_splash->width;
        uint32_t h = early_splash->height;
        char buf[20];
        int i = 0;
        if (w == 0) buf[i++] = '0';
        else {
            int digits = 0;
            uint32_t temp = w;
            while (temp > 0) { temp /= 10; digits++; }
            for (int j = digits - 1; j >= 0; j--) {
                temp = w;
                for (int k = 0; k < j; k++) temp /= 10;
                buf[i++] = '0' + (temp % 10);
            }
        }
        buf[i++] = 'x';
        if (h == 0) buf[i++] = '0';
        else {
            int digits = 0;
            uint32_t temp = h;
            while (temp > 0) { temp /= 10; digits++; }
            for (int j = digits - 1; j >= 0; j--) {
                temp = h;
                for (int k = 0; k < j; k++) temp /= 10;
                buf[i++] = '0' + (temp % 10);
            }
        }
        buf[i] = '\0';
        gfx_print(buf);
        gfx_print("\n");
        
        // Show memory pool stats while PNG allocation is active
        gfx_print("\n");
        memory_pool_print_all_stats();
        gfx_print("\n");
        
        // Display title showing PNG loaded
        video_subsystem_splash_title("PNG CHECKERBOARD LOADED!", 
                                    (rgb_color_t){255, 255, 0, 255},  // Yellow text
                                    (rgb_color_t){255, 0, 255, 255}); // Magenta bg
        
        png_free(early_splash);
        SERIAL_LOG("PNG test complete - image freed\n");
        gfx_print("PNG test complete - image freed\n");
    } else {
        SERIAL_LOG("FAILED: Could not load PNG image\n");
        gfx_print("FAILED: Could not load PNG image\n");
    }
    SERIAL_LOG("===EARLY PNG TEST END===\n");
    gfx_print("===EARLY PNG TEST END===\n");
    
    // Initialize filesystem subsystem
    SERIAL_LOG("[KERNEL] About to init filesystem subsystem\n");
    filesystem_subsystem_init(NULL);
    SERIAL_LOG("[KERNEL] Filesystem subsystem initialized\n");
    gfx_print("Filesystem subsystem initialized.\n");
    
    SERIAL_LOG("[KERNEL] About to initialize VFS\n");
    gfx_print("DEBUG: About to initialize VFS...\n");
    
    // Initialize VFS and mount RAM disk
    vfs_init();
    SERIAL_LOG("[KERNEL] VFS init completed\n");
    gfx_print("DEBUG: VFS init completed successfully.\n");
    gfx_print("VFS initialized and RAM disk mounted.\n");
    
    // Initialize ISO9660 filesystem (CD-ROM driver will be initialized after PCI)
    SERIAL_LOG("[KERNEL] ===== INITIALIZING ISO9660 FILESYSTEM =====\n");
    iso9660_init();
    SERIAL_LOG("[KERNEL] ISO9660 init completed\n");
    
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
    pci_init();
    gfx_print("===SKIPPING MOUSE INIT===\n");
    // mouse_init();  // TEMPORARILY DISABLED - causes crash/jump
    gfx_print("===CONTINUING AFTER MOUSE===\n");
    
    // Initialize CD-ROM driver AFTER PCI init (needed to discover IDE/ATAPI)
    SERIAL_LOG("[KERNEL] ===== INITIALIZING CD-ROM DRIVER (POST-PCI) =====\n");
    gfx_print("Initializing CD-ROM driver...\n");
    cdrom_init();
    SERIAL_LOG("[KERNEL] CD-ROM init completed\n");
    
    // Now mount ISO9660 after PCI has discovered devices
    SERIAL_LOG("[KERNEL] ===== MOUNTING ISO9660 FILESYSTEM =====\n");
    gfx_print("Mounting ISO9660 filesystem...\n");
    blockdev_t* cdrom_dev = blockdev_find("cdrom0");
    SERIAL_LOG("[KERNEL] Searching for cdrom0 device\n");
    if (cdrom_dev) {
        SERIAL_LOG("[KERNEL] CD-ROM device found, mounting\n");
        iso9660_mount(cdrom_dev, "/cdrom");
        SERIAL_LOG("[KERNEL] ISO9660 mount completed\n");
        gfx_print("ISO9660 filesystem mounted successfully\n");
    } else {
        SERIAL_LOG("[KERNEL] WARNING: CD-ROM device NOT found\n");
        gfx_print("WARNING: CD-ROM device not found\n");
    }
    
    // Display PNG splash screen before task manager takes over
    gfx_print("===LOADING PNG SPLASH===\n");
    png_image_t* splash_image = load_splash_image();
    if (splash_image) {
        gfx_print("PNG: Splash loaded successfully!\n");
        // TODO: Actually display the image on screen
        // For now, just show success message
        video_subsystem_splash_title("PNG Splash Pattern Loaded!", 
                                    (rgb_color_t){255, 255, 255, 255}, 
                                    (rgb_color_t){0, 180, 180, 255});
        png_free(splash_image);
    } else {
        gfx_print("PNG: Failed to load splash\n");
    }
    gfx_print("===PNG SPLASH DONE===\n");
    
    /* Initialize task manager */
    gfx_print("Initializing task manager...\n");
    task_manager_init();
    gfx_print("Task manager initialization complete.\n");
    
    /* Initialize and demonstrate advanced scheduler architecture */
    gfx_print("Initializing advanced modular scheduler...\n");
    if (scheduler_demo_init() == 0) {
        gfx_print("Advanced scheduler initialized successfully.\n");
        
        gfx_print("Running scheduler demonstration...\n");
        if (scheduler_demo_run() == 0) {
            gfx_print("Scheduler demonstration completed successfully.\n");
            gfx_print("Quantum OS modular subsystem architecture is operational!\n");
        } else {
            gfx_print("Scheduler demonstration failed.\n");
        }
    } else {
        gfx_print("Failed to initialize advanced scheduler.\n");
    }
    
    SERIAL_LOG("[KERNEL] Scheduler demo completed, continuing...\n");
    SERIAL_LOG("[KERNEL] About to print AFTER_SCHEDULER_DEMO\n");
    gfx_print("===AFTER_SCHEDULER_DEMO===\n");
    SERIAL_LOG("[KERNEL] Printed AFTER_SCHEDULER_DEMO\n");
    
    // now that keyboard and IDT are initialized so
    // modal popups can use the interrupt-driven keyboard buffer.
    __asm__ volatile("sti");
    SERIAL_LOG("[KERNEL] Interrupts enabled\n");
    //message_box_push("System initialized. Ready."); 
    gfx_print("Keyboard driver initialized.\n");
    SERIAL_LOG("[KERNEL] Keyboard message printed\n");
    
    // Draw PNG splash screen from ISO9660 filesystem
    SERIAL_LOG("[KERNEL] ===== LOADING PNG FROM CD-ROM =====\n");
    gfx_print("Loading PNG splash image from CD-ROM...\n");
    
    // Get framebuffer pointer and dimensions
    SERIAL_LOG("[KERNEL] Getting framebuffer info\n");
    uint32_t* fb = video_subsystem_get_framebuffer();
    uint32_t fb_width, fb_height;
    video_subsystem_get_resolution(&fb_width, &fb_height);
    SERIAL_LOG("[KERNEL] Framebuffer obtained\n");
    
    if (fb) {
        SERIAL_LOG("[KERNEL] Framebuffer valid, using static PNG buffer\n");
        
        // Clear framebuffer to black first
        for (uint32_t i = 0; i < fb_width * fb_height; i++) {
            fb[i] = 0xFF000000; // Black with full alpha
        }
        SERIAL_LOG("[KERNEL] Framebuffer cleared to black\n");
        
        // Allocate PNG buffer from VIDEO subsystem pool (2MB)
        uint8_t* png_buffer = (uint8_t*)memory_pool_alloc_large(SUBSYSTEM_VIDEO, 2048 * 1024, 0);
        if (!png_buffer) {
            gfx_print("Failed to allocate PNG buffer\n");
            return 0;
        }
        
        SERIAL_LOG("[KERNEL] Reading PNG from ISO9660\n");
        // Read the splash screen from ISO9660
        int bytes_read = iso9660_read_file("/SPLASH.PNG", png_buffer, 2048 * 1024, 0);
        SERIAL_LOG("[KERNEL] iso9660_read_file returned: ");
        char bytes_str[16];
        int temp_val = bytes_read;
        int idx_val = 0;
        if (temp_val <= 0) {
            bytes_str[idx_val++] = temp_val < 0 ? '-' : '0';
            if (temp_val < 0) temp_val = -temp_val;
        }
        if (temp_val > 0) {
            char digits_val[16];
            int d_val = 0;
            while (temp_val > 0) { digits_val[d_val++] = '0' + (temp_val % 10); temp_val /= 10; }
            for (int i = d_val-1; i >= 0; i--) bytes_str[idx_val++] = digits_val[i];
        }
        bytes_str[idx_val] = '\0';
        SERIAL_LOG(bytes_str);
        SERIAL_LOG("\n");
        
        if (bytes_read > 0) {
            SERIAL_LOG("[KERNEL] PNG file loaded from CD-ROM!\n");
            gfx_print("PNG file loaded from CD-ROM!\n");
            // Decode PNG to framebuffer
            png_decode_to_framebuffer((const uint8_t*)png_buffer, bytes_read, fb, fb_width, fb_height);
            SERIAL_LOG("[KERNEL] PNG decoded to framebuffer\n");
            gfx_print("PNG splash displayed!\n");
            
            // Show memory pool stats while allocations are active
            gfx_print("\n");
            memory_pool_print_all_stats();
        } else {
            SERIAL_LOG("[KERNEL] Failed to read PNG from CD-ROM\n");
            gfx_print("Failed to read PNG from CD-ROM, using fallback pattern\n");
            // Use embedded fallback
            load_splash_to_framebuffer(fb, fb_width, fb_height);
        }
        
        // Free PNG buffer
        memory_pool_free(SUBSYSTEM_VIDEO, png_buffer);
        
        // Wait for keypress, then clear splash and show text
        gfx_print("Press any key to continue...\n");
        
        // Simple wait for keyboard input
        extern volatile bool key_pressed;
        key_pressed = false;
        while (!key_pressed) {
            __asm__ volatile("hlt");
        }
        
        // Clear screen and restore text display
        video_subsystem_clear_screen();
        gfx_print("Splash cleared. Continuing boot...\n");
    }
    
    // Display video subsystem debug info
    video_subsystem_debug_info();
    
    gfx_print("Video subsystem test complete.\n");
    
    // Demo filesystem subsystem functionality
    gfx_print("Testing filesystem subsystem...\n");
    
    // Register some demo files that exist in our RAM disk
    filesystem_register_file("boot_config", "/ramdisk/config.txt", FILE_TYPE_CONFIG);
    filesystem_register_file("kernel_log", "/ramdisk/kernel.log", FILE_TYPE_TEXT);
    filesystem_register_file("system_info", "/ramdisk/sysinfo.txt", FILE_TYPE_TEXT);
    
    // Create and register an in-memory test file to demonstrate working functionality
    static char test_data[] = "QuantumOS Test File\nFilesystem Subsystem Working!\n";
    filesystem_register_file("test_memory_file", "memory://test.txt", FILE_TYPE_TEXT);
    
    // Use the filesystem subsystem API to set file data
    if (filesystem_set_file_data("test_memory_file", test_data, sizeof(test_data) - 1)) {
        gfx_print("Created in-memory test file successfully.\n");
    } else {
        gfx_print("Failed to create in-memory test file.\n");
    }
    
    // Test file operations with real VFS files
    if (filesystem_load_file("boot_config")) {
        gfx_print("Successfully loaded boot_config from RAM disk!\n");
        
        // Display actual file content
        size_t file_size;
        char* file_data = (char*)filesystem_get_file_data("boot_config", &file_size);
        if (file_data && file_size > 0) {
            gfx_print("File content preview: ");
            // Show first 40 characters
            for (size_t i = 0; i < file_size && i < 40; i++) {
                if (file_data[i] == '\n') {
                    gfx_print(" [LF] ");
                } else if (file_data[i] >= 32 && file_data[i] <= 126) {
                    char temp[2] = {file_data[i], '\0'};
                    gfx_print(temp);
                }
            }
            gfx_print("...\n");
        }
    } else {
        gfx_print("Failed to load boot_config.\n");
    }
    
    // Test file lookup functionality
    registered_file_t* test_file = filesystem_lookup_file("system_info");
    if (test_file) {
        gfx_print("Found registered file: system_info\n");
        
        // Try to load and show this file too
        if (filesystem_load_file("system_info")) {
            size_t info_size;
            char* info_data = (char*)filesystem_get_file_data("system_info", &info_size);
            if (info_data) {
                gfx_print("System info loaded successfully!\n");
            }
        }
    } else {
        gfx_print("Failed to find system_info (unexpected)\n");
    }
    
    // Test accessing the in-memory file data
    size_t data_size;
    void* file_data = filesystem_get_file_data("test_memory_file", &data_size);
    if (file_data) {
        gfx_print("Successfully accessed test file data (");
        char size_str[16];
        char* size_ptr = size_str + 15;
        *size_ptr = '\0';
        int size_val = (int)data_size;
        if (size_val == 0) {
            *--size_ptr = '0';
        } else {
            while (size_val > 0) {
                *--size_ptr = '0' + (size_val % 10);
                size_val /= 10;
            }
        }
        gfx_print(size_ptr);
        gfx_print(" bytes)\n");
    }
    
    // Display filesystem subsystem statistics
    filesystem_subsystem_stats_t fs_stats;
    filesystem_subsystem_get_stats(&fs_stats);
    gfx_print("Filesystem Stats - Registered files: ");
    
    // Convert numbers to strings for display
    char reg_files_str[16], loaded_files_str[16];
    // Simple number to string conversion
    int reg_files = (int)fs_stats.total_files_registered;
    int loaded_files = (int)fs_stats.total_files_loaded;
    
    // Convert registered files count
    char* reg_ptr = reg_files_str + 15;
    *reg_ptr = '\0';
    if (reg_files == 0) {
        *--reg_ptr = '0';
    } else {
        while (reg_files > 0) {
            *--reg_ptr = '0' + (reg_files % 10);
            reg_files /= 10;
        }
    }
    
    // Convert loaded files count  
    char* loaded_ptr = loaded_files_str + 15;
    *loaded_ptr = '\0';
    if (loaded_files == 0) {
        *--loaded_ptr = '0';
    } else {
        while (loaded_files > 0) {
            *--loaded_ptr = '0' + (loaded_files % 10);
            loaded_files /= 10;
        }
    }
    
    gfx_print(reg_ptr);
    gfx_print(", Loaded files: ");
    gfx_print(loaded_ptr);
    gfx_print("\n");
    
    gfx_print("Filesystem subsystem test complete.\n");

    gfx_print("===END OF KERNEL_MAIN===\n");
    
    __asm__ volatile("sti");
    while(1) {
        __asm__ volatile("hlt");
    }

    return 0;
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


