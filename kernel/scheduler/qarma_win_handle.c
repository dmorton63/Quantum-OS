#include "qarma_win_handle.h"
#include "../graphics/framebuffer.h"

void qarma_winhandler_update_all(QARMA_WIN_HANDLER *handler, QARMA_TICK_CONTEXT *ctx) {
    for (uint32_t i = 0; i < handler->count; ++i) {
        QARMA_WIN_HANDLE *win = handler->windows[i];
        if (win && win->update) {
            win->update(win, ctx);
            if (win->dirty) {
                fb_mark_dirty();
                win->dirty = false;
            }
        }
    }
}

void splash_update(QARMA_WIN_HANDLE *self, QARMA_TICK_CONTEXT *ctx) {
    static float splash_linger = 0.0f;
    splash_linger += ctx->delta_time;

    if (splash_linger < 60.0f) return;  // Wait at least 2 seconds

    if (self->flags & QARMA_WIN_FADE_OUT) {
        float fade_speed = 0.5f;
        self->background.a -= (uint8_t)(ctx->delta_time * 255 * fade_speed);
        if (self->background.a <= 0) {
            self->destroy(self);
        } else {
            self->dirty = true;
        }
    }
}

void qarma_winhandler_add(QARMA_WIN_HANDLER *handler, QARMA_WIN_HANDLE *win) {
    if (handler->count >= QARMA_MAX_MODULES) {
        // Optional: log overflow or trace rejection
        return;
    }

    handler->windows[handler->count++] = win;

    // Mark the screen as dirty to force a redraw
    fb_mark_dirty();

    // Optional: trace memory tag
    if (win->owner && win->owner->name) {
        gfx_printf("[QARMA] Window added: %s (App: %s)\n", QARMA_MEM_TAG_WIN, win->owner->name);
    } else {
        gfx_printf("[QARMA] Window added: %s\n", QARMA_MEM_TAG_WIN);
    }
}

void qarma_winhandler_render_all(QARMA_WIN_HANDLER *handler) {
    for (uint32_t i = 0; i < handler->count; ++i) {
        QARMA_WIN_HANDLE *win = handler->windows[i];
        if (win && (win->flags & QARMA_FLAG_VISIBLE)) {
            if (win->render) {
                win->render(win);  // Draw into its framebuffer
            }
        }
    }
    fb_compose();  // Composite all window framebuffers to screen
}

void splash_app_update(QARMA_APP_HANDLE *self, QARMA_TICK_CONTEXT *ctx) {
    if (!self || !self->main_window || !self->main_window->update) return;
    self->main_window->update(self->main_window, ctx);
}