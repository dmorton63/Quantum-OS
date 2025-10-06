/**
 * QuantumOS - Interrupt System
 * Handles IRQ routing and system initialization
 */

#include "../core/kernel.h"
#include "../graphics/graphics.h"
#include "../keyboard/keyboard.h"
#include "../core/io.h"
// Central interrupt handler invoked by irq_common_stub
void interrupt_handler(uint32_t interrupt_number, uint32_t error_code) {
    interrupt_number &= 0xFF;

    switch (interrupt_number) {
        case 33: { // IRQ1: Keyboard
            regs_t regs = { .int_no = interrupt_number, .err_code = error_code };
            keyboard_handler(&regs);
            break;
        }

        // Future IRQs can be added here
        case 208: // Spurious interrupt - acknowledge and ignore
            // Send EOI to both PICs
            outb(0xA0, 0x20); // EOI to slave PIC
            outb(0x20, 0x20); // EOI to master PIC
            break;
            
        default:
            gfx_print("Unhandled interrupt: ");
            gfx_print_decimal(interrupt_number);
            gfx_print(" (err=");
            gfx_print_decimal(error_code);
            gfx_print(")\n");
            
            // Send EOI for hardware interrupts (32-47)
            if (interrupt_number >= 32 && interrupt_number < 48) {
                if (interrupt_number >= 40) {
                    outb(0xA0, 0x20); // EOI to slave PIC
                }
                outb(0x20, 0x20); // EOI to master PIC
            }
            break;
    }
}

// Initializes GDT and IDT — must be called before enabling interrupts
void interrupts_system_init(void) {
    gfx_print("Setting up interrupt system...\n");

    gdt_init();  // Segment setup
    idt_init();  // IDT + IRQ stubs

    gfx_print("GDT and IDT setup complete.\n");
}

// Optional quantum-aware interrupt enhancements
void quantum_interrupts_init(void) {
    gfx_print("Quantum-aware interrupt handling enabled.\n");
}