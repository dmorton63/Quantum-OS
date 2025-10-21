#pragma once
#include "../core/stdtools.h"
#include "../core/memory.h"
#include "qarma_schedtypedefs.h"

typedef enum {
    QARMA_FLAG_NONE       = 0x00,
    QARMA_FLAG_VISIBLE    = 0x01,
    QARMA_FLAG_MODAL      = 0x02,
    QARMA_FLAG_FADE_OUT   = 0x04,
    QARMA_FLAG_FOCUSED    = 0x08,
    // Add more flags as needed
} QARMA_WINDOW_FLAGS;

typedef struct QARMA_WIN_HANDLE {
    uint32_t id;                        // Unique window ID
    QARMA_COORD position;              // Top-left corner
    QARMA_DIMENSION size;              // Width and height
    QARMA_COLOR background;            // Background color
    uint32_t flags;                    // Modal, fade-out, visibility
    void *content;                     // Pointer to app-specific content
    struct QARMA_APP_HANDLE *owner;   // Owning app
    bool active;                       // Is this window currently focused?
    bool dirty;                        // Needs redraw
    void (*render)(struct QARMA_WIN_HANDLE *self);   // Render function
    void (*update)(struct QARMA_WIN_HANDLE *self, QARMA_TICK_CONTEXT *ctx); // Tick update
    void (*destroy)(struct QARMA_WIN_HANDLE *self);  // Destruction ritual
} QARMA_WIN_HANDLE;

typedef struct QARMA_WIN_HANDLER {
    QARMA_WIN_HANDLE *windows[QARMA_MAX_MODULES];  // Array of window pointers
    uint32_t count;                                // Active window count

    void (*add)(struct QARMA_WIN_HANDLER *handler, QARMA_WIN_HANDLE *win);            // Register a new window
    void (*remove)(struct QARMA_WIN_HANDLER *handler, uint32_t id);                   // Remove by ID
    void (*update_all)(struct QARMA_WIN_HANDLER *handler, QARMA_TICK_CONTEXT *ctx);   // Called by scheduler
    void (*render_all)(struct QARMA_WIN_HANDLER *handler);                          // Called by video subsystem
    void (*destroy_all)(struct QARMA_WIN_HANDLER *handler);                         // Called on shutdown
} QARMA_WIN_HANDLER;

typedef struct QARMA_APP_HANDLE {
    uint32_t id;                          // Unique app ID
    const char *name;                     // App name or title
    QARMA_WIN_HANDLE *main_window;        // Primary window glyph
    void *state;                          // App-specific state or context
    void (*init)(struct QARMA_APP_HANDLE *self);     // Initialization ritual
    void (*update)(struct QARMA_APP_HANDLE *self, QARMA_TICK_CONTEXT *ctx); // Tick update
    void (*handle_event)(struct QARMA_APP_HANDLE *self, QARMA_EVENT *event); // Event dispatch
    void (*shutdown)(struct QARMA_APP_HANDLE *self); // Destruction ritual
} QARMA_APP_HANDLE;


void qarma_winhandler_update_all(QARMA_WIN_HANDLER *handler, QARMA_TICK_CONTEXT *ctx);

void splash_update(QARMA_WIN_HANDLE *self, QARMA_TICK_CONTEXT *ctx);

void qarma_winhandler_add(QARMA_WIN_HANDLER *handler, QARMA_WIN_HANDLE *win);

void qarma_winhandler_render_all(QARMA_WIN_HANDLER *handler);

void splash_app_update(QARMA_APP_HANDLE *self, QARMA_TICK_CONTEXT *ctx);
