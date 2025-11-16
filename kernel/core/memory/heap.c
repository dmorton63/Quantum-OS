#include "heap.h"
#include "../memory/vmm/vmm.h"

#define STATIC_HEAP_SIZE (20 * 1024 * 1024)  // 20MB static heap (enough for ~10 PNG buffers)
static uint8_t static_heap[STATIC_HEAP_SIZE] __attribute__((aligned(4096)));
static uint8_t* heap_base = NULL;
static uint8_t* heap_top  = NULL;
static bool heap_initialized = false;

void heap_init(void) {
    heap_base = static_heap;
    heap_top  = heap_base;
    heap_initialized = true;
}

void* heap_alloc(size_t size) {
    if (!heap_initialized) {
        heap_init(); // Auto-initialize if needed
    }
    
    size = (size + 7) & ~7; // align to 8 bytes
    
    // Check if we have enough space
    if (heap_top + size > heap_base + STATIC_HEAP_SIZE) {
        return NULL; // Out of heap space
    }
    
    void* result = heap_top;
    heap_top += size;
    
    // Zero the allocated memory to prevent garbage data
    uint8_t* ptr = (uint8_t*)result;
    for (size_t i = 0; i < size; i++) {
        ptr[i] = 0;
    }
    
    // Debug output for large allocations (like framebuffer backing store)
    if (size > 1024 * 1024) {
        // Only log large allocations to avoid spam
        // This will catch the framebuffer backing store allocation
    }
    
    return result;
}

void heap_free(void* ptr) {
    // no-op for now - memory is not reclaimed
    // With 20MB heap, we can allocate ~10 PNG buffers before running out
    (void)ptr;
}

void* heap_alloc_aligned(size_t size, size_t alignment) {
    if (!heap_initialized) {
        heap_init(); // Auto-initialize if needed
    }
    
    // Align the current heap top to the required alignment
    uintptr_t current = (uintptr_t)heap_top;
    uintptr_t aligned = (current + alignment - 1) & ~(alignment - 1);
    
    // Calculate total size needed
    size_t padding = aligned - current;
    size_t total_size = padding + size;
    
    // Check if we have enough space
    if (heap_top + total_size > heap_base + STATIC_HEAP_SIZE) {
        return NULL; // Out of heap space
    }
    
    // Update heap_top and return aligned pointer
    heap_top += total_size;
    
    // Zero the allocated memory to prevent garbage data
    uint8_t* ptr = (uint8_t*)aligned;
    for (size_t i = 0; i < size; i++) {
        ptr[i] = 0;
    }
    
    return (void*)aligned;
}



