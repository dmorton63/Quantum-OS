#ifndef MOUSE_H 
#define MOUSE_H
#include "../stdtools.h"
typedef struct {
    int x;                // Current X position on screen
    int y;                // Current Y position on screen
    int dx;               // Delta X since last update
    int dy;               // Delta Y since last update
    bool left_pressed;    // Left button state
    bool right_pressed;   // Right button state
    bool middle_pressed;  // Middle button (if supported)
    bool scroll_up;       // Scroll wheel up event
    bool scroll_down;     // Scroll wheel down event
} mouse_state_t;


mouse_state_t* get_mouse_state(void);
void update_mouse_state_from_packet(uint8_t packet[3]);

#endif 