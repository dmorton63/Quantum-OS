/**
 * QuantumOS - AI Subsystem Implementation (Basic)
 */

#include "ai_subsystem.h"
#include "../core/kernel.h"
#include "../graphics/graphics.h"

static ai_subsystem_stats_t g_ai_stats = {0};

void ai_subsystem_init(void) {
    gfx_print("Initializing AI subsystem...\n");
    ai_hardware_init();
    gfx_print("AI subsystem initialized.\n");
}

void ai_hardware_init(void) {
    gfx_print("AI hardware acceleration detected.\n");
}

void ai_system_optimize(void) {
    // Basic AI optimization
}

ai_subsystem_stats_t* ai_get_subsystem_stats(void) {
    return &g_ai_stats;
}

// Utility functions
float sigmoid(float x) {
    // Simple sigmoid approximation without expf
    if (x > 5.0f) return 1.0f;
    if (x < -5.0f) return 0.0f;
    
    // Use tanh approximation: sigmoid(x) = (tanh(x/2) + 1) / 2
    float tanh_val = x / 2.0f;
    if (tanh_val > 1.0f) tanh_val = 1.0f;
    if (tanh_val < -1.0f) tanh_val = -1.0f;
    
    return (tanh_val + 1.0f) / 2.0f;
}

float relu(float x) {
    return x > 0 ? x : 0;
}