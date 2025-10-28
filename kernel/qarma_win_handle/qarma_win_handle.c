#include "qarma_win_handle.h"

static uint32_t next_window_id = 1;

uint32_t qarma_generate_window_id(void) {
    return next_window_id++;
}

void qarma_win_assign_vtable(QARMA_WIN_HANDLE* win, QARMA_WIN_VTABLE* vtable) {
    win->vtable = vtable;
}