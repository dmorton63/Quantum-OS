#include "../kernel_types.h"
#include "../core/kernel.h"
#include "../graphics/graphics.h"

// IDT Entry Structure
struct idt_entry {
    uint16_t base_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t flags;
    uint16_t base_high;
} PACKED;

// IDT Pointer Structure
struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} PACKED;

// IDT Entries
static struct idt_entry idt_entries[256];
static struct idt_ptr idt_pointer;

// External assembly functions
extern void idt_flush(uint32_t idt_ptr_addr);

// IRQ stubs (vectors 32–47)
extern void (*irq_stubs[16])(void);

// Default handler for unmapped interrupts
extern void irqdefault(void);

/** Set an IDT entry */
static void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags) {
    idt_entries[num].base_low  = base & 0xFFFF;
    idt_entries[num].base_high = (base >> 16) & 0xFFFF;
    idt_entries[num].selector  = selector;
    idt_entries[num].zero      = 0;
    idt_entries[num].flags     = flags | 0x60; // User mode access
}

/** Initialize the IDT */
void idt_init(void) {
    idt_pointer.limit = sizeof(struct idt_entry) * 256 - 1;
    idt_pointer.base  = (uint32_t)&idt_entries;

    memset(&idt_entries, 0, sizeof(idt_entries));

    // Phase 1: IRQs (hardware interrupts mapped to 32–47)
    for (int i = 0; i < 16; i++) {
        idt_set_gate(32 + i, (uint32_t)irq_stubs[i], 0x08, 0x8E);
    }

    // Phase 2: Default handler for all other vectors
    for (int i = 0; i < 32; i++) {
        if (i == 32 || i == 33) continue; // Skip IRQ0 and IRQ1 if already mapped
        idt_set_gate(i, (uint32_t)irqdefault, 0x08, 0x8E);
    }
    for (int i = 48; i < 256; i++) {
        idt_set_gate(i, (uint32_t)irqdefault, 0x08, 0x8E);
    }

    // Final invocation: load the IDT
    idt_flush((uint32_t)&idt_pointer);
}