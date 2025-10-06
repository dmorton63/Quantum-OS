/**
 * QuantumOS - GDT (Global Descriptor Table) Setup
 * 
 * Proper GDT initialization for protected mode operation
 */

#include "../kernel_types.h"
#include "../core/kernel.h"
#include "../graphics/graphics.h"

// GDT Entry Structure
struct gdt_entry {
    uint16_t limit_low;    // Lower 16 bits of limit
    uint16_t base_low;     // Lower 16 bits of base
    uint8_t base_middle;   // Next 8 bits of base
    uint8_t access;        // Access flags
    uint8_t granularity;   // Granularity flags and upper 4 bits of limit
    uint8_t base_high;     // Last 8 bits of base
} PACKED;

// GDT Pointer Structure
struct gdt_ptr {
    uint16_t limit;        // Size of GDT
    uint32_t base;         // Address of GDT
} PACKED;

// GDT Entries
static struct gdt_entry gdt_entries[5];
static struct gdt_ptr gdt_pointer;

// External assembly function to load GDT
extern void gdt_flush(uint32_t gdt_ptr_addr);

/**
 * Set a GDT entry
 */
static void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt_entries[num].base_low = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high = (base >> 24) & 0xFF;

    gdt_entries[num].limit_low = (limit & 0xFFFF);
    gdt_entries[num].granularity = (limit >> 16) & 0x0F;

    gdt_entries[num].granularity |= gran & 0xF0;
    gdt_entries[num].access = access;
}

/**
 * Initialize the GDT
 */
void gdt_init(void) {
    gfx_print("Setting up GDT...\n");
    
    gdt_pointer.limit = (sizeof(struct gdt_entry) * 5) - 1;
    gdt_pointer.base = (uint32_t)&gdt_entries;

    // Null descriptor (required)
    gdt_set_gate(0, 0, 0, 0, 0);

    // Kernel code segment
    // Base = 0, Limit = 4GB, Access = 0x9A (present, ring 0, executable, readable)
    // Granularity = 0xCF (4KB granularity, 32-bit)
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    // Kernel data segment
    // Base = 0, Limit = 4GB, Access = 0x92 (present, ring 0, writable)
    // Granularity = 0xCF (4KB granularity, 32-bit)
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    // User code segment (for future use)
    // Access = 0xFA (present, ring 3, executable, readable)
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);

    // User data segment (for future use)
    // Access = 0xF2 (present, ring 3, writable)
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    // Load the new GDT
    gdt_flush((uint32_t)&gdt_pointer);
    
    gfx_print("GDT initialized successfully.\n");
}