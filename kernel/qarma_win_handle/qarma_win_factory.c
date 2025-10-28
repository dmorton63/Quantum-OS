#include "qarma_win_factory.h"
//#include "../window_types/qarma_splash_app.h"  // For splash-specific init
#include "qarma_window_manager.h"
#include "../splash_app/qarma_splash_window.h"
#include "../graphics/graphics.h"

QARMA_WIN_HANDLE* qarma_win_create(QARMA_WIN_TYPE type, const char* title, uint32_t flags) {
    gfx_printf("[qarma_win_factory] Creating generic window of type %d with title '%s'\n", type, title ? title : "(null)");
    QARMA_WIN_HANDLE* win = malloc(sizeof(QARMA_WIN_HANDLE));
    if (!win) return NULL;

    win->id = qarma_generate_window_id();
    win->type = type;
    win->flags = flags;
    win->x = 100;
    win->y = 100;
    win->size.height = 640;
    win->size.width = 480;
    win->alpha = 1.0f;
    win->title = title;
    win->vtable = NULL;   // No behavior assigned yet
    win->traits = NULL;   // No traits attached
    gfx_printf("Created window ID %u of type %u\n", win->id, win->type);
    qarma_window_manager.add_window(&qarma_window_manager, win,"Win Factory");
    return win;
}

QARMA_WIN_HANDLE* qarma_win_create_archetype(uint32_t archetype_id, const char* title, uint32_t flags) {
    switch (archetype_id) {
        case QARMA_WIN_TYPE_SPLASH:
            return (QARMA_WIN_HANDLE*) splash_window_create(title, flags);

        // case QARMA_WIN_TYPE_MODAL:
        //      return (QARMA_WIN_HANDLE*) modal_window_create(title, flags);

        // case QARMA_WIN_TYPE_DIALOG:
        //      return (QARMA_WIN_HANDLE*) dialog_window_create(title, flags);
        case QARMA_WIN_TYPE_CLOCK_OVERLAY:
            return qarma_win_create(QARMA_WIN_TYPE_CLOCK_OVERLAY, title, flags);

        case QARMA_WIN_TYPE_GENERIC:
            
        default:
            panic("qarma_win_create_archetype: unknown archetype ID");
            return NULL;
    }
}