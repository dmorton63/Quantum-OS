#include "qarma_window_manager.h"
#include "panic.h"
#include "../graphics/graphics.h"

QARMA_WINDOW_MANAGER qarma_window_manager;

void add_window(QARMA_WINDOW_MANAGER* mgr, QARMA_WIN_HANDLE* win, const char* caller) {
    gfx_printf("[qarma_win_factory] Creating window: ID %u, type %d, title '%s'\n", win->id, win->type, win->title ? win->title : "(null)");
    gfx_printf("Caller: %s\n", caller);
    //gfx_printf("[add_window] Called from %u → ID %u, type %d\n", caller, win->id, win->type);
    if(win->title == "") {
        gfx_printf("Warning: Window title is empty string.\n");
    } else {
        gfx_printf("Window title: '%s'\n", win->title);
        if (!mgr || !win) {
            panic("add_window: manager or window is NULL");
            return;
        }

        if (!win->vtable) {
            panic("add_window: window vtable is NULL");
            return;
        }

        if ((win->type == QARMA_WIN_TYPE_SPLASH || (win->flags & QARMA_FLAG_FADE_OUT)) && !win->traits) {
            panic("add_window: splash window missing traits");
            return;
        }

        if (mgr->count >= QARMA_MAX_WINDOWS) {
            panic("add_window: window manager overflow");
            return;
        }
    }
    mgr->windows[mgr->count++] = win;
}

static void update_all(QARMA_WINDOW_MANAGER* mgr, QARMA_TICK_CONTEXT* ctx) {
    for (uint32_t i = 0; i < mgr->count; i++) {
        QARMA_WIN_HANDLE* win = mgr->windows[i];
        if (win && win->vtable && win->vtable->update) {
            win->vtable->update(win, ctx);
        }
    }
}

static void render_all(QARMA_WINDOW_MANAGER* mgr) {
    for (uint32_t i = 0; i < mgr->count; i++) {
        QARMA_WIN_HANDLE* win = mgr->windows[i];
        if (win && (win->flags & QARMA_FLAG_VISIBLE) && win->vtable && win->vtable->render) {
            win->vtable->render(win);
        }
    }
}

static void destroy_all(QARMA_WINDOW_MANAGER* mgr) {
    for (uint32_t i = 0; i < mgr->count; i++) {
        QARMA_WIN_HANDLE* win = mgr->windows[i];
        if (win && win->vtable && win->vtable->destroy) {
            win->vtable->destroy(win);
        }
        mgr->windows[i] = NULL;
    }
    mgr->count = 0;
}

static void remove_window(QARMA_WINDOW_MANAGER* mgr, uint32_t id) {
    for (uint32_t i = 0; i < mgr->count; i++) {
        QARMA_WIN_HANDLE* win = mgr->windows[i];
        if (win && win->id == id) {
            if (win->vtable && win->vtable->destroy) {
                win->vtable->destroy(win);
            }
            for (uint32_t j = i; j < mgr->count - 1; j++) {
                mgr->windows[j] = mgr->windows[j + 1];
            }
            mgr->windows[--mgr->count] = NULL;
            break;
        }
    }
}

void qarma_window_manager_init() {
    qarma_window_manager.count = 0;
    qarma_window_manager.add_window = add_window;
    qarma_window_manager.remove_window = remove_window;
    qarma_window_manager.update_all = update_all;
    qarma_window_manager.render_all = render_all;
    qarma_window_manager.destroy_all = destroy_all;
}

