// Uncomment to enable specific debug channels
#define DEBUG_SERIAL
#define DEBUG_BOOTLOG
#define DEBUG_GRAPHICS
#define DEBUG_MEMORY
typedef enum {
    VERBOSITY_SILENT,
    VERBOSITY_MINIMAL,
    VERBOSITY_VERBOSE
} verbosity_level_t;

extern verbosity_level_t g_verbosity;


#ifdef DEBUG_SERIAL
    #define SERIAL_LOG(msg) do { \
        if (g_verbosity >= VERBOSITY_VERBOSE) serial_debug(msg); \
    } while (0)

    #define SERIAL_LOG_MIN(msg) do { \
        if (g_verbosity >= VERBOSITY_MINIMAL) serial_debug(msg); \
    } while (0)

    #define SERIAL_LOG_HEX(prefix, val) do { \
        if (g_verbosity >= VERBOSITY_VERBOSE) { \
            serial_debug(prefix); \
            serial_debug_hex(val); \
            serial_debug("\n"); \
        } \
    } while (0)

    #define SERIAL_LOG_DEC(prefix, val) do { \
        if (g_verbosity >= VERBOSITY_VERBOSE) { \
            serial_debug(prefix); \
            serial_debug_decimal(val); \
            serial_debug("\n"); \
        } \
    } while (0)
#else
    #define SERIAL_LOG(msg) ((void)(msg))
    #define SERIAL_LOG_MIN(msg) ((void)(msg))
    #define SERIAL_LOG_HEX(prefix, val) ((void)(prefix), (void)(val))
    #define SERIAL_LOG_DEC(prefix, val) ((void)(prefix), (void)(val))
#endif

#ifdef DEBUG_BOOTLOG
    #define BOOT_LOG(msg) boot_log_push(msg)
    #define BOOT_LOG_HEX(prefix, val) boot_log_push_hex(prefix, val)
    #define BOOT_LOG_DEC(prefix, val) boot_log_push_decimal(prefix, val)
#else
    #define BOOT_LOG(msg) ((void)(msg))
    #define BOOT_LOG_HEX(prefix, val) ((void)(prefix), (void)(val))
    #define BOOT_LOG_DEC(prefix, val) ((void)(prefix), (void)(val))
#endif





#ifdef DEBUG_GRAPHICS
    #define GFX_LOG(msg) do { \
        if (g_verbosity >= VERBOSITY_VERBOSE) gfx_print(msg); \
    } while (0)

    #define GFX_LOG_MIN(msg) do { \
        if (g_verbosity >= VERBOSITY_MINIMAL) gfx_print(msg); \
    } while (0)

    #define GFX_LOG_HEX(prefix, val) do { \
        if (g_verbosity >= VERBOSITY_VERBOSE) { \
            gfx_print(prefix); \
            gfx_print_hex(val); \
            gfx_print("\n"); \
        } \
    } while (0)

    #define GFX_LOG_DEC(prefix, val) do { \
        if (g_verbosity >= VERBOSITY_VERBOSE) { \
            gfx_print(prefix); \
            gfx_print_decimal(val); \
            gfx_print("\n"); \
        } \
    } while (0)

#else
    #define GFX_LOG(msg) ((void)(msg))
    #define GFX_LOG_MIN(msg) ((void)(msg))
    #define GFX_LOG_HEX(prefix, val) ((void)(prefix), (void)(val))
    #define GFX_LOG_DEC(prefix, val) ((void)(prefix), (void)(val))
#endif
