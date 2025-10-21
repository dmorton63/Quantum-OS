// Process management API
#pragma once
#include "qarma_schedtypedefs.h"


int qarma_create_process(void (*main)(QARMA_TICK_CONTEXT *, QARMA_PROCESS *), int priority, void *context);
void qarma_stop_process(int pid);

void qarma_dispatch_event(void *event_ptr);

void qarma_shutdown();

//uint64_t get_system_time();

uint64_t get_system_time();

void qarma_register_module(void *module, const char *name);

void qarma_tick();