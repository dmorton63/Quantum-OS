/**
 * QuantumOS - Memory Management Header
 * 
 * Function declarations for memory management subsystems
 */

#ifndef MEMORY_H
#define MEMORY_H

#include "../kernel_types.h"

// Page size and alignment
#define PAGE_SIZE 0x1000  // 4KB pages
#define ALIGNED(x) __attribute__((aligned(x)))

// Physical Memory Manager (PMM)
void pmm_init(void);
void pmm_mark_region_free(uint32_t start_addr, uint32_t length);
void pmm_mark_region_used(uint32_t start_addr, uint32_t length);
uint32_t pmm_alloc_page(void);
void pmm_free_page(uint32_t addr);
void pmm_print_stats(void);

// Virtual Memory Manager (VMM)
void vmm_init(void);
void vmm_map_page(uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags);
uint32_t vmm_alloc_region(uint32_t size);
void vmm_free_region(uint32_t virtual_addr, uint32_t size);
bool vmm_map_framebuffer(uint32_t fb_physical_addr, uint32_t fb_size);

// Heap Management
void heap_init(void);
void* heap_alloc(size_t size);
void heap_free(void* ptr);

// Memory utility functions
void kernel_delay(uint32_t count);

// String functions (implemented in string.c)
void* memset(void* ptr, int value, size_t size);
void* memcpy(void* dest, const void* src, size_t size);
size_t strlen(const char* str);
char* strcpy(char* dest, const char* src);
int strcmp(const char* str1, const char* str2);

#endif // MEMORY_H