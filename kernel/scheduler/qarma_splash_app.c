#include "qarma_splash_app.h"
#include "../graphics/framebuffer.h"
#include "../core/memory.h"

QARMA_WIN_HANDLER global_win_handler = {
    .windows = {0},
    .count = 0,
    .add = qarma_winhandler_add,
    .remove = qarma_winhandler_remove,
    .update_all = qarma_winhandler_update_all,
    .render_all = qarma_winhandler_render_all,
    .destroy_all = NULL
};

void splash_init(QARMA_APP_HANDLE *self) {
    QARMA_WIN_HANDLE *win = (QARMA_WIN_HANDLE*)malloc(sizeof(QARMA_WIN_HANDLE));
    win->id = 101;
    win->position = (QARMA_COORD){30, 10};
    win->size = (QARMA_DIMENSION){640, 480};
    win->background = (QARMA_COLOR){0, 0, 0, 255}; // Black, full alpha
    win->flags = QARMA_WIN_VISIBLE | QARMA_WIN_FADE_OUT;
    win->owner = self;
    win->active = true;
    win->dirty = true;
    win->update = splash_window_update;
    win->render = splash_window_render;
    win->destroy = splash_window_destroy;

    self->main_window = win;
    qarma_winhandler_add(&global_win_handler, win);
}

QARMA_APP_HANDLE splash_app = {
    .id = 1,
    .name = "Splash Ritual",
    .main_window = NULL,
    .state = NULL,
    .init = splash_init,
    .update = splash_app_update,
    .handle_event = splash_event,
    .shutdown = splash_shutdown
};


void splash_window_render(QARMA_WIN_HANDLE *self) {
    uint32_t color = self->background.r |
                     (self->background.g << 8) |
                     (self->background.b << 16) |
                     (self->background.a << 24);

    //gfx_printf("Rendering window ID %u with color 0x%08X\n", self->id, color);

    fb_draw_rect(self->position.x, self->position.y,
                 self->size.width, self->size.height,
                 color);
}

void splash_window_update(QARMA_WIN_HANDLE *self, QARMA_TICK_CONTEXT *ctx) {
    float fade_speed = 0.5f;
    uint8_t g = self->background.g;

    int decay = (int)(ctx->delta_time * 255 * fade_speed);
    //gfx_printf("Tick %u: Green=%u, Decay=%d\n", ctx->tick_count, g, decay);

    if (g > 0) {
        g = (g > decay) ? g - decay : 0;
        self->background.g = g;
        self->dirty = true;
    } else {
        self->destroy(self);
    }
}
// void splash_update(QARMA_APP_HANDLE *self, QARMA_TICK_CONTEXT *ctx) {
//     if (self->main_window && self->main_window->update) {
//         self->main_window->update(self->main_window, ctx);
//     }
// }

void splash_shutdown(QARMA_APP_HANDLE *self) {
    if (self->main_window && self->main_window->destroy) {
        self->main_window->destroy(self->main_window);
    }
    gfx_printf("[QARMA] Splash app shutdown.\n");
}

void splash_event(QARMA_APP_HANDLE *self, QARMA_EVENT *event) {
    (void)self;
    (void)event;
    // placeholder for future ritual
}

void splash_window_destroy(QARMA_WIN_HANDLE *self) {
    // Remove from global_win_handler
    for (uint32_t i = 0; i < global_win_handler.count; ++i) {
        if (global_win_handler.windows[i] == self) {
            // Shift remaining windows down
            for (uint32_t j = i; j < global_win_handler.count - 1; ++j) {
                global_win_handler.windows[j] = global_win_handler.windows[j + 1];
            }
            global_win_handler.windows[--global_win_handler.count] = NULL;
            break;
        }
    }

    // Clear splash_app reference
    if (splash_app.main_window == self) {
        splash_app.main_window = NULL;
    }

    // Free memory
    free(self);
        fb_mark_dirty();
}


void qarma_winhandler_remove(QARMA_WIN_HANDLER *handler, uint32_t id) {
    for (uint32_t i = 0; i < handler->count; ++i) {
        QARMA_WIN_HANDLE *win = handler->windows[i];
        if (win && win->id == id) {
            if (win->destroy) {
                win->destroy(win);
            }

            // Shift remaining windows down
            for (uint32_t j = i; j < handler->count - 1; ++j) {
                handler->windows[j] = handler->windows[j + 1];
            }

            handler->windows[--handler->count] = NULL;

            gfx_printf("[QARMA] Window ID %u removed from handler.\n", id);
                        fb_mark_dirty();
            return;
        }
    }

    gfx_printf("[QARMA] Window ID %u not found.\n", id);
}