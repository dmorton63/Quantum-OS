#include "pci.h"
#include "../graphics/graphics.h"
#include "../keyboard/command.h"
//#include "../config.h"

// Scan PCI bus 0..255, slot 0..31, func 0..7 and print vendor/device IDs
void pci_scan_and_print(void) {
    gfx_print("Scanning PCI bus...\n");
    GFX_LOG("Starting PCI bus scan...\n");
    for (uint8_t bus = 0; bus < 1; ++bus) { // start with bus 0 for now
        for (uint8_t slot = 0; slot < 32; ++slot) {
            for (uint8_t func = 0; func < 8; ++func) {
                uint16_t vendor = pci_read_config_word(bus, slot, func, 0x00);
                if (vendor == 0xFFFF) continue; // no device
                uint16_t device = pci_read_config_word(bus, slot, func, 0x02);
                GFX_LOG_DEC("Found device at bus ", bus);
                GFX_LOG_DEC(" slot ", slot);
                GFX_LOG_DEC(" func ", func);
                GFX_LOG_HEX(" vendor=0x", vendor);
                GFX_LOG_HEX(" device=0x", device);

                gfx_print("PCI ");
                gfx_print_decimal(bus);
                gfx_print(":");
                gfx_print_decimal(slot);
                gfx_print(":");
                gfx_print_decimal(func);
                gfx_print(" vendor=0x");
                gfx_print_hex(vendor);
                gfx_print(" device=0x");
                gfx_print_hex(device);
                gfx_print("\n");
            }
        }
    }
}
