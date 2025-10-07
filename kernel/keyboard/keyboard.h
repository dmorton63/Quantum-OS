#pragma once

#include "../../kernel/kernel_types.h"  // For regs_t
#include "keyboard_types.h"          // For keyboard_state_t, key_modifiers_t







bool keyboard_init(void);
//void keyboard_handler(regs_t* regs);
void keyboard_send_eoi(uint32_t int_no);
void keyboard_process_scancode(uint8_t scancode);
void keyboard_handle_key_press(uint8_t scancode);
void keyboard_handle_key_release(uint8_t scancode);
void keyboard_handle_ctrl_combo(uint8_t scancode);
void keyboard_clear_buffer(void);
bool keyboard_ctrl_pressed(void);
bool is_printable_key(uint8_t scancode);
char scancode_to_ascii(uint8_t scancode, bool shift, bool caps);
void keyboard_add_to_buffer(char c);
keyboard_state_t* get_keyboard_state(void);
