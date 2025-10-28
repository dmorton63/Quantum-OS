#include "qarma_splash_window.h"
#include "../graphics/framebuffer.h"
#include "../qarma_win_handle/qarma_window_manager.h"
#include "../core/memory.h"

// Extended splash glyph
typedef struct QARMA_SPLASH_HANDLE {
    QARMA_WIN_HANDLE base;
    float fade_speed;
} QARMA_SPLASH_HANDLE;

// Forward declarations
static void splash_update(QARMA_WIN_HANDLE* self, QARMA_TICK_CONTEXT* ctx);
static void splash_render(QARMA_WIN_HANDLE* self);
static void splash_destroy(QARMA_WIN_HANDLE* self);

// Behavior scroll
static QARMA_WIN_VTABLE splash_vtable = {
    .init = NULL,
    .update = splash_update,
    .render = splash_render,
    .destroy = splash_destroy
};

// Glyph summoner
QARMA_SPLASH_HANDLE* splash_window_create(const char* title, uint32_t flags) {
    QARMA_SPLASH_HANDLE* splash = malloc(sizeof(QARMA_SPLASH_HANDLE));
    if (!splash) return NULL;

    QARMA_WIN_HANDLE* win = &splash->base;
    win->id = qarma_generate_window_id();
    win->type = QARMA_WIN_TYPE_SPLASH;
    win->flags = flags;
    win->x = 30;
    win->y = 10;
    win->size = (QARMA_DIMENSION){640, 480};
    win->alpha = 1.0f;
    win->title = title;
    win->background = (QARMA_COLOR){0, 0, 0, 255};
    win->vtable = &splash_vtable;
    win->traits = splash;
    win->buffer_size = win->size;
    win->pixel_buffer = malloc(win->buffer_size.width * win->buffer_size.height * sizeof(uint32_t));
    if (win->pixel_buffer) {
        memset(win->pixel_buffer, 0, win->buffer_size.width * win->buffer_size.height * sizeof(uint32_t));
    }

    splash->fade_speed = 1.0f;
    qarma_window_manager.add_window(&qarma_window_manager, win,"Splash Window");
    return splash;
}

// Fade logic
static void splash_update(QARMA_WIN_HANDLE* self, QARMA_TICK_CONTEXT* ctx) {
    QARMA_SPLASH_HANDLE* splash = (QARMA_SPLASH_HANDLE*) self;
    int decay = (int)(ctx->delta_time * 255 * splash->fade_speed);
    uint8_t a = self->background.a;

    if (a > 0) {
        self->background.a = (a > decay) ? a - decay : 0;
        self->dirty = true;
    } else {
        self->vtable->destroy(self);
    }
}

// Render logic
static void splash_render(QARMA_WIN_HANDLE* self) {
    fb_draw_rect_to_buffer(
        self->pixel_buffer,
        self->buffer_size,
        0, 0,
        self->buffer_size,
        self->background
    );
}

// Destruction ritual
static void splash_destroy(QARMA_WIN_HANDLE* self) {
    qarma_window_manager.remove_window(&qarma_window_manager, self->id);

    if (self->pixel_buffer) {
        free(self->pixel_buffer);
        self->pixel_buffer = NULL;
    }

    free(self);
    fb_mark_dirty();
}

