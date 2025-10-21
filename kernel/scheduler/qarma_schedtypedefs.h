
#define QARMA_MAX_PROCESSES 32
#pragma once
#include "../core/stdtools.h"

#define QARMA_TICK_RATE 60
#define QARMA_MAX_MODULES 32

#define QARMA_EVENT_INPUT     0x01
#define QARMA_EVENT_RENDER    0x02
#define QARMA_EVENT_MEMORY    0x03
#define QARMA_EVENT_SHUTDOWN  0xFF

#define QARMA_WIN_MODAL       0x01
#define QARMA_WIN_FADE_OUT    0x02
#define QARMA_WIN_VISIBLE     0x04

#define QARMA_MEM_TAG_WIN     "QWIN"
#define QARMA_MEM_TAG_APP     "APP"
#define QARMA_MEM_TAG_EVENT   "EVENT"

typedef struct QARMA_SCHEDULER {
    uint64_t tick_count;           // Total ticks since boot
    uint32_t tick_rate;            // Ticks per second (e.g. 60)
    bool running;                  // Is the scheduler active?

    void (*tick)();                // Main update loop
    void (*register_module)(void *module, const char *name);
    void (*dispatch_event)(void *event);   // Optional: event routing
    void (*sync)();                // Optional: sync subsystems
    void (*shutdown)();            // Graceful teardown
} QARMA_SCHEDULER;

typedef struct {
    uint64_t tick_count;
    float delta_time;         // Time since last tick
    float uptime_seconds;
} QARMA_TICK_CONTEXT;

typedef struct {
    uint32_t type;
    void *payload;
    const char *origin;
    const char *target;
} QARMA_EVENT;

typedef struct {
    const char *name;
    void (*update)(QARMA_TICK_CONTEXT *ctx);
    void (*shutdown)();
    void (*handle_event)(QARMA_EVENT *event);  // NEW: event handler
} QARMA_MODULE;

typedef struct {
    int x;
    int y;
} QARMA_COORD;

typedef struct {
    int width;
    int height;
} QARMA_DIMENSION;

typedef struct {
    uint8_t r, g, b, a;
} QARMA_COLOR;

// Process states
typedef enum {
    QARMA_PROCESS_RUNNING,
    QARMA_PROCESS_WAITING,
    QARMA_PROCESS_STOPPED
} QARMA_PROCESS_STATE;

typedef struct QARMA_PROCESS {
    int pid;
    QARMA_PROCESS_STATE state;
    int priority;
    void (*main)(QARMA_TICK_CONTEXT *ctx, struct QARMA_PROCESS *self);
    void *context; // user-defined process context
} QARMA_PROCESS;

