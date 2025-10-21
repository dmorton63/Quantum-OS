#include "mouse.h"
#include "../../config.h"
#include "../graphics/framebuffer.h"
#include "../core/io.h"

static uint8_t packet[3];
static uint8_t packet_index = 0;
static mouse_state_t mouse_state = {0};
int fb_width = 800;
int fb_height = 600;

void mouse_handler() {
    packet[packet_index++] = inb(0x60);
    //SERIAL_LOG("Mouse interrupt received\n");
    // gfx_print("Mouse interrupt, byte ");
    // gfx_print_decimal(packet_index);
    // gfx_print(": ");
    // gfx_print_hex(packet[packet_index - 1]);
    // gfx_print("\n");
    if (packet_index == 3) {
        packet_index = 0;
        update_mouse_state_from_packet(packet);
    }

    // Send EOI to both PICs
    outb(0xA0, 0x20); // slave PIC
    outb(0x20, 0x20); // master PIC
}

// void parse_mouse_packet(uint8_t packet[3]) {
//     int x = (int8_t)packet[1]; // signed movement
//     int y = (int8_t)packet[2];
//     bool left = packet[0] & 0x01;
//     bool right = packet[0] & 0x02;
//     bool middle = packet[0] & 0x04;
//     //gfx_print("Mouse packet: ");
//     // Apply movement, update cursor, etc.
// }



mouse_state_t* get_mouse_state(void) {
    return &mouse_state;
}

void update_mouse_state_from_packet(uint8_t packet[3]) {
    int dx = (int8_t)packet[1];
    int dy = (int8_t)packet[2];

    mouse_state.dx = dx;
    mouse_state.dy = dy;

    mouse_state.x += dx;
    mouse_state.y -= dy;  // Y is typically inverted

    // Clamp to screen bounds
    if (mouse_state.x < 0) mouse_state.x = 0;
    if (mouse_state.y < 0) mouse_state.y = 0;
    if (mouse_state.x >= (int)fb_width)  mouse_state.x = fb_width - 1;
    if (mouse_state.y >= (int)fb_height) mouse_state.y = fb_height - 1;

    // Update button states
    mouse_state.left_pressed   = packet[0] & 0x01;
    mouse_state.right_pressed  = packet[0] & 0x02;
    mouse_state.middle_pressed = packet[0] & 0x04;

    // Scroll wheel bits (if supported—extend as needed)
    mouse_state.scroll_up   = false;
    mouse_state.scroll_down = false;

    // Optional: dispatch events
    // dispatch_mouse_event(mouse_state.x, mouse_state.y, mouse_state.left_pressed, ...);

    // Optional: mark cursor region dirty for redraw
    // mark_cursor_dirty(mouse_state.x, mouse_state.y);
}
