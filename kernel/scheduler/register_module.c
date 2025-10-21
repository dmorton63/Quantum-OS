#include "register_module.h"
#include "../core/stdtools.h"
#include "../core/timer.h"
#include "../core/string.h"


#define QARMA_MAX_MODULES 32
typedef struct {
    QARMA_MODULE *module;
    const char *name;
} qarma_module_entry_t;
static qarma_module_entry_t qarma_modules[QARMA_MAX_MODULES];
static int qarma_module_count = 0;


QARMA_SCHEDULER scheduler = {
    .tick_rate = QARMA_TICK_RATE,
    .running = true,
    .tick = qarma_tick,
    .register_module = qarma_register_module,
    .dispatch_event = qarma_dispatch_event,
    .sync = NULL,
    .shutdown = qarma_shutdown
};

// Process table
static QARMA_PROCESS qarma_processes[QARMA_MAX_PROCESSES];
static int qarma_process_count = 0;

// Create a new process
int qarma_create_process(void (*main)(QARMA_TICK_CONTEXT *, QARMA_PROCESS *), int priority, void *context) {
    if (qarma_process_count >= QARMA_MAX_PROCESSES) return -1;
    int pid = qarma_process_count;
    QARMA_PROCESS *proc = &qarma_processes[qarma_process_count++];
    proc->pid = pid;
    proc->state = QARMA_PROCESS_RUNNING;
    proc->priority = priority;
    proc->main = main;
    proc->context = context;
    return pid;
}

// Stop a process
void qarma_stop_process(int pid) {
    if (pid < 0 || pid >= qarma_process_count) return;
    qarma_processes[pid].state = QARMA_PROCESS_STOPPED;
}

// Tick processes
static void qarma_tick_processes(QARMA_TICK_CONTEXT *ctx) {
    for (int i = 0; i < qarma_process_count; i++) {
        QARMA_PROCESS *proc = &qarma_processes[i];
        if (proc->state == QARMA_PROCESS_RUNNING && proc->main) {
            proc->main(ctx, proc);
        }
    }
}
uint64_t get_system_time() {
    return get_system_timer(1000).millis;
}

void qarma_register_module(void *module, const char *name) {
    if (qarma_module_count >= QARMA_MAX_MODULES) return;
    qarma_modules[qarma_module_count].module = (QARMA_MODULE *)module;
    qarma_modules[qarma_module_count].name = name;
    qarma_module_count++;
}

void qarma_tick() {
    static uint64_t last_tick_time = 0;
    uint64_t now = get_system_time();
    float delta = (now - last_tick_time) / 1000.0f;
    last_tick_time = now;

    QARMA_TICK_CONTEXT ctx = {
        .tick_count = now / (1000 / QARMA_TICK_RATE),
        .delta_time = delta,
        .uptime_seconds = now / 1000.0f
    };

    // Update modules
    for (int i = 0; i < qarma_module_count; i++) {
        QARMA_MODULE *mod = qarma_modules[i].module;
        if (mod && mod->update) {
            mod->update(&ctx);
        }
    }

    // Update processes
    qarma_tick_processes(&ctx);
}

void qarma_dispatch_event(void *event_ptr) {
    QARMA_EVENT *event = (QARMA_EVENT *)event_ptr;
    for (int i = 0; i < qarma_module_count; i++) {
        QARMA_MODULE *mod = qarma_modules[i].module;
        const char *name = qarma_modules[i].name;
        if (mod && mod->handle_event) {
            if (!event->target || (name && strcmp(name, event->target) == 0)) {
                mod->handle_event(event);
            }
        }
    }
}

void qarma_shutdown() {
    scheduler.running = false;

    for (int i = 0; i < qarma_module_count; i++) {
        QARMA_MODULE *mod = qarma_modules[i].module;
        if (mod && mod->shutdown) {
            mod->shutdown();
        }
    }

    // Dispatch a global shutdown event (broadcast)
    QARMA_EVENT shutdown_event = {
        .type = QARMA_EVENT_SHUTDOWN,
        .payload = NULL,
        .origin = "scheduler",
        .target = NULL
    };
    qarma_dispatch_event(&shutdown_event);
}