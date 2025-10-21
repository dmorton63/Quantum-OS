#include "pci.h"
#include "../graphics/graphics.h"
#include "../keyboard/command.h"


// Scan PCI bus 0..255, slot 0..31, func 0..7 and print vendor/device IDs
void pci_scan_and_print(void) {
    gfx_print("Scanning PCI bus...\n");
    gfx_print("Starting PCI bus scan...\n");
    for (uint8_t bus = 0; bus < 1; ++bus) { // start with bus 0 for now
        for (uint8_t slot = 0; slot < 32; ++slot) {
            for (uint8_t func = 0; func < 8; ++func) {
                uint16_t vendor = pci_read_config_word(bus, slot, func, 0x00);
                if (vendor == 0xFFFF) continue; // no device
                uint16_t device = pci_read_config_word(bus, slot, func, 0x02);
                char buf[128];
                sprintf(buf, "Found device at bus %d slot %d func %d vendor=0x%04x device=0x%04x\n", bus, slot, func, vendor, device);
                for(int i=0; buf[i] != '\0'; i++)
                    gfx_putchar(buf[i]);
                // gfx_print("Found device at bus ");
                // gfx_print_decimal(bus);
                // gfx_print(" slot ");
                // gfx_print_decimal(slot);
                // gfx_print(" func ");
                // gfx_print_decimal(func);
                // gfx_print(" vendor=0x");
                // gfx_print_hex(vendor);
                // gfx_print(" device=0x");
                // gfx_print_hex(device);

                // gfx_print("PCI ");
                // gfx_print_decimal(bus);
                // gfx_print(":");
                // gfx_print_decimal(slot);
                // gfx_print(":");
                // gfx_print_decimal(func);
                // gfx_print(" vendor=0x");
                // gfx_print_hex(vendor);
                // gfx_print(" device=0x");
                // gfx_print_hex(device);
                // gfx_print("\n");
            }
        }
    }
}
