/**
 * QuantumOS - Basic Memory Management and String Functions
 */

#include "../core/kernel.h"
#include "../graphics/graphics.h"
#include "multiboot.h"
#include "memory.h"
#include "../config.h"


// Forward declarations for serial debug functions
extern void serial_debug(const char* msg);
extern void serial_debug_hex(uint32_t value);

// Page flags
#define PAGE_PRESENT    0x1
#define PAGE_WRITE      0x2
#define PAGE_USER       0x4
#define PAGE_SIZE       0x1000  // 4KB pages

// Physical Memory Manager
#define PMM_MAX_PAGES   32768   // Support up to 128MB (32768 * 4KB)
#define PMM_BITMAP_SIZE (PMM_MAX_PAGES / 8)  // Bitmap for page allocation

#define HEAP_SIZE (1024 * 1024)  // 1MB
static char heap[HEAP_SIZE];
static char* heap_end = heap;


static uint8_t pmm_bitmap[PMM_BITMAP_SIZE];
static uint32_t pmm_total_pages = 0;
static uint32_t pmm_used_pages = 0;
static uint32_t pmm_free_pages = 0;
static bool pmm_initialized = false;

// Simple page directory for basic mapping
// static uint32_t page_directory[1024] __attribute__((aligned(4096)));
// static uint32_t page_table_0[1024] __attribute__((aligned(4096)));

// Simple page directory and table for framebuffer mapping
static uint32_t page_directory[1024] __attribute__((aligned(4096)));
static uint32_t framebuffer_page_table[1024] __attribute__((aligned(4096)));
static bool paging_enabled = false;

// Virtual memory management state
static bool vmm_initialized = false;
static uint32_t vmm_next_virtual_addr = 0xC0000000; // Start virtual allocations at 3GB

void vmm_init(void) {
    gfx_print("Initializing Virtual Memory Manager...\n");
    
    // Initialize page directory and basic identity mapping
    for (int i = 0; i < 1024; i++) {
        page_directory[i] = 0; // Clear all entries
    }
    
    // Map low memory (first 4MB) - identity mapping for kernel
    for (int i = 0; i < 1024; i++) {
        framebuffer_page_table[i] = (i * 0x1000) | PAGE_PRESENT | PAGE_WRITE;
    }
    page_directory[0] = (uint32_t)framebuffer_page_table | PAGE_PRESENT | PAGE_WRITE;
    
    vmm_initialized = true;
    gfx_print("Virtual memory manager initialized (identity mapped + framebuffer).\n");
    // Print information about virtual allocation space
    GFX_LOG_HEX("Virtual allocation space starts at: ", vmm_next_virtual_addr);
}

// Map framebuffer memory region
bool vmm_map_framebuffer(uint32_t fb_physical_addr, uint32_t fb_size) {
    SERIAL_LOG_HEX("Mapping framebuffer: phys=", fb_physical_addr);
    SERIAL_LOG_HEX(" size=", fb_size);
    SERIAL_LOG("\n");
    
    // For QEMU, framebuffer is typically in high memory (0xFD000000+)
    // We need to set up page tables to access it
    
    if (fb_physical_addr < 0x100000) {
        SERIAL_LOG("FB address in low memory, already accessible\n");
        return true; // Low memory is already mapped
    }
    
    // Check if address is in QEMU framebuffer range
    if (fb_physical_addr >= 0xE0000000 && fb_physical_addr < 0xFFE00000) {
        // Check if paging is already enabled
        if (paging_enabled) {
            SERIAL_LOG("Paging already enabled, framebuffer should be accessible\n");
            return true;
        }
        
        SERIAL_LOG("Setting up page table for framebuffer...\n");
        
        // Set up basic paging to map the framebuffer
        // We'll use identity mapping: virtual address = physical address
        
        // Initialize page directory
        for (int i = 0; i < 1024; i++) {
            page_directory[i] = 0; // Clear all entries
        }
        
        // Map low memory (first 4MB) - identity mapping
        for (int i = 0; i < 1024; i++) {
            framebuffer_page_table[i] = (i * 0x1000) | PAGE_PRESENT | PAGE_WRITE;
        }
        page_directory[0] = (uint32_t)framebuffer_page_table | PAGE_PRESENT | PAGE_WRITE;
        
        // Map framebuffer region
        uint32_t fb_page_start = fb_physical_addr >> 12; // Convert to page number
        uint32_t fb_page_count = (fb_size + 0xFFF) >> 12; // Round up to pages
        uint32_t fb_pde_index = fb_physical_addr >> 22; // Page directory entry index
        
        SERIAL_LOG_HEX("FB pages: start=", fb_page_start);
        SERIAL_LOG_HEX(" count=", fb_page_count);
        SERIAL_LOG_HEX(" pde_idx=", fb_pde_index);
        SERIAL_LOG("\n");
        
        // For simplicity, create a page table for the framebuffer region
        // This is a basic implementation - normally you'd allocate page tables dynamically
        static uint32_t fb_page_table[1024] __attribute__((aligned(4096)));
        
        // Map the framebuffer pages
        uint32_t fb_base_page = (fb_physical_addr >> 22) << 10; // Base page in this 4MB region
        for (uint32_t i = 0; i < 1024; i++) {
            uint32_t page_addr = ((fb_base_page + i) << 12);
            fb_page_table[i] = page_addr | PAGE_PRESENT | PAGE_WRITE;
        }
        
        // Install page table in directory
        page_directory[fb_pde_index] = (uint32_t)fb_page_table | PAGE_PRESENT | PAGE_WRITE;
        
        // Enable paging
        uint32_t page_dir_phys = (uint32_t)page_directory;
        
        __asm__ volatile (
            "mov %0, %%cr3\n"           // Set page directory
            "mov %%cr0, %%eax\n"
            "or $0x80000000, %%eax\n"  // Set PG bit
            "mov %%eax, %%cr0\n"
            : : "r" (page_dir_phys) : "eax"
        );
        
        paging_enabled = true;
        SERIAL_LOG("Paging enabled with framebuffer mapping\n");
        return true;
    }
    
    gfx_print("FB address out of supported range\n");
    return false;
}

// Map a single page
void vmm_map_page(uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags) {
    if (!vmm_initialized) {
        GFX_LOG_MIN("VMM: Cannot map page - VMM not initialized");
        return;
    }
    
    // For now, disable page mapping to prevent crashes
    // This needs proper paging setup first
    GFX_LOG_MIN("VMM: Page mapping disabled for stability");
    (void)virtual_addr; (void)physical_addr; (void)flags;
    return;
    
    /*
    uint32_t page_dir_idx = virtual_addr >> 22;
    uint32_t page_table_idx = (virtual_addr >> 12) & 0x3FF;
    
    GFX_LOG_HEX("VMM: Mapping virtual ", virtual_addr);
    GFX_LOG_HEX("VMM: to physical ", physical_addr);
    
    // Check if page table exists
    if (!(page_directory[page_dir_idx] & PAGE_PRESENT)) {
        // Allocate a new page table
        uint32_t page_table_phys = pmm_alloc_page();
        if (page_table_phys == 0) {
            gfx_print("VMM: Failed to allocate page table\n");
            return;
        }
        
        // Clear the page table
        uint32_t* page_table = (uint32_t*)page_table_phys;
        for (int i = 0; i < 1024; i++) {
            page_table[i] = 0;
        }
        
        // Install page table in directory
        page_directory[page_dir_idx] = page_table_phys | PAGE_PRESENT | PAGE_WRITE | flags;
    }
    
    // Get page table and map the page
    uint32_t* page_table = (uint32_t*)(page_directory[page_dir_idx] & ~0xFFF);
    page_table[page_table_idx] = physical_addr | PAGE_PRESENT | flags;
    
    // Invalidate TLB for this page
    __asm__ volatile("invlpg (%0)" :: "r"(virtual_addr) : "memory");
    */
}

// Allocate virtual memory region
uint32_t vmm_alloc_region(uint32_t size) {
    (void)size; // Suppress unused parameter warning
    if (!vmm_initialized) {
        GFX_LOG_MIN("VMM: Cannot allocate region - VMM not initialized");
        return 0;
    }
    
    // For now, disable virtual allocation to prevent crashes
    // TODO: Enable when paging is properly set up
    GFX_LOG_MIN("VMM: Virtual allocation disabled for stability");
    return 0;
    
    /*
    // Align size to page boundary
    size = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    uint32_t num_pages = size / PAGE_SIZE;
    
    uint32_t virtual_start = vmm_next_virtual_addr;
    vmm_next_virtual_addr += size;
    
    gfx_print("VMM: Allocated virtual region ");
    gfx_print_hex(virtual_start);
    gfx_print("-");
    gfx_print_hex(virtual_start + size);
    gfx_print(" (");
    gfx_print_decimal(num_pages);
    gfx_print(" pages)\n");
    
    // Allocate physical pages and map them
    for (uint32_t i = 0; i < num_pages; i++) {
        uint32_t phys_page = pmm_alloc_page();
        if (phys_page == 0) {
            gfx_print("VMM: Failed to allocate physical page ");
            gfx_print_decimal(i);
            gfx_print(" of ");
            gfx_print_decimal(num_pages);
            gfx_print("\n");
            return 0;
        }
        
        uint32_t virtual_page = virtual_start + (i * PAGE_SIZE);
        vmm_map_page(virtual_page, phys_page, PAGE_WRITE);
    }
    
    return virtual_start;
    */
}

// Free virtual memory region
void vmm_free_region(uint32_t virtual_addr, uint32_t size) {
    if (!vmm_initialized) {
        return;
    }
    
    // Align to page boundaries
    size = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    uint32_t num_pages = size / PAGE_SIZE;
    
    gfx_print("VMM: Freeing virtual region ");
    gfx_print_hex(virtual_addr);
    gfx_print("-");
    gfx_print_hex(virtual_addr + size);
    gfx_print("\n");
    
    for (uint32_t i = 0; i < num_pages; i++) {
        uint32_t virtual_page = virtual_addr + (i * PAGE_SIZE);
        uint32_t page_dir_idx = virtual_page >> 22;
        uint32_t page_table_idx = (virtual_page >> 12) & 0x3FF;
        
        if (page_directory[page_dir_idx] & PAGE_PRESENT) {
            uint32_t* page_table = (uint32_t*)(page_directory[page_dir_idx] & ~0xFFF);
            if (page_table[page_table_idx] & PAGE_PRESENT) {
                uint32_t phys_addr = page_table[page_table_idx] & ~0xFFF;
                pmm_free_page(phys_addr);
                page_table[page_table_idx] = 0;
                
                // Invalidate TLB
                __asm__ volatile("invlpg (%0)" :: "r"(virtual_page) : "memory");
            }
        }
    }
}

// PMM helper functions
static inline void pmm_set_page(uint32_t page_num) {
    uint32_t byte_idx = page_num / 8;
    uint32_t bit_idx = page_num % 8;
    if (byte_idx < PMM_BITMAP_SIZE) {
        pmm_bitmap[byte_idx] |= (1 << bit_idx);
    }
}

static inline void pmm_clear_page(uint32_t page_num) {
    uint32_t byte_idx = page_num / 8;
    uint32_t bit_idx = page_num % 8;
    if (byte_idx < PMM_BITMAP_SIZE) {
        pmm_bitmap[byte_idx] &= ~(1 << bit_idx);
    }
}

static inline bool pmm_is_page_set(uint32_t page_num) {
    uint32_t byte_idx = page_num / 8;
    uint32_t bit_idx = page_num % 8;
    if (byte_idx < PMM_BITMAP_SIZE) {
        return (pmm_bitmap[byte_idx] & (1 << bit_idx)) != 0;
    }
    return true; // Assume used if out of bounds
}

void *malloc(size_t size)
{
    Block* curr = freeList;
    while (curr) {
        if (curr->free && curr->size >= size) {
            curr->free = 0;
            return (void*)(curr + 1);
        }
        curr = curr->next;
    }

    Block* newBlock = sbrk(size + BLOCK_SIZE);
    if (newBlock == (void*)-1) return NULL;

    newBlock->size = size;
    newBlock->next = freeList;
    newBlock->free = 0;
    freeList = newBlock;

    return (void*)(newBlock + 1);

}



void free(void *ptr)
{
    if (!ptr) return;
    Block* block = (Block*)ptr - 1;
    block->free = 1;

}

void pmm_init(void)
{
    // Don't print during boot - can cause serial issues
    // gfx_print("Initializing Physical Memory Manager...\n");
    
    // Clear the bitmap (mark all pages as used initially)
    memset(pmm_bitmap, 0xFF, PMM_BITMAP_SIZE);
    
    pmm_total_pages = PMM_MAX_PAGES;  // Set to maximum initially
    pmm_used_pages = PMM_MAX_PAGES;   // All marked as used initially
    pmm_free_pages = 0;
    pmm_initialized = true;
    
    // Don't print during boot
    // gfx_print("PMM bitmap initialized\n");
}

void pmm_mark_region_free(uint32_t start_addr, uint32_t length) {
    if (!pmm_initialized) return;
    
    uint32_t start_page = start_addr / PAGE_SIZE;
    uint32_t end_page = (start_addr + length) / PAGE_SIZE;
    
    // Avoid serial debug during boot - causes issues
    // gfx_print("PMM: Marking region free\n");
    
    for (uint32_t page = start_page; page < end_page && page < PMM_MAX_PAGES; page++) {
        if (pmm_is_page_set(page)) {
            pmm_clear_page(page);
            pmm_used_pages--;
            pmm_free_pages++;
        }
        
        // Update total pages count
        if (page >= pmm_total_pages) {
            pmm_total_pages = page + 1;
        }
    }
}

void pmm_mark_region_used(uint32_t start_addr, uint32_t length) {
    if (!pmm_initialized) return;
    
    uint32_t start_page = start_addr / PAGE_SIZE;
    uint32_t end_page = (start_addr + length) / PAGE_SIZE;
    
    for (uint32_t page = start_page; page < end_page && page < PMM_MAX_PAGES; page++) {
        if (!pmm_is_page_set(page)) {
            pmm_set_page(page);
            pmm_used_pages++;
            pmm_free_pages--;
        }
    }
}

uint32_t pmm_alloc_page(void) {
    if (!pmm_initialized) return 0;
    
    for (uint32_t page = 0; page < PMM_MAX_PAGES; page++) {
        if (!pmm_is_page_set(page)) {
            pmm_set_page(page);
            pmm_used_pages++;
            pmm_free_pages--;
            return page * PAGE_SIZE;
        }
    }
    
    GFX_LOG_MIN("PMM: Out of physical memory!");
    return 0; // No free pages
}

void pmm_free_page(uint32_t addr) {
    if (!pmm_initialized) return;
    
    uint32_t page = addr / PAGE_SIZE;
    if (page < PMM_MAX_PAGES && pmm_is_page_set(page)) {
        pmm_clear_page(page);
        pmm_used_pages--;
        pmm_free_pages++;
    }
}

void pmm_print_stats(void) {
    gfx_print("=== Physical Memory Manager Statistics ===\n");
    gfx_print("Total pages: ");
    gfx_print_decimal(pmm_total_pages);
    gfx_print("\n");
    gfx_print("Used pages: ");
    gfx_print_decimal(pmm_used_pages);
    gfx_print(" (");
    gfx_print_decimal((pmm_used_pages * PAGE_SIZE) / 1024);
    gfx_print(" KB)\n");
    
    gfx_print("Free pages: ");
    gfx_print_decimal(pmm_free_pages);
    gfx_print(" (");
    gfx_print_decimal((pmm_free_pages * PAGE_SIZE) / 1024);
    gfx_print(" KB)\n");
}

void heap_init(void) {
    gfx_print("Kernel heap initialized.\n");
}

// Simple heap allocator (placeholder implementation)
void* heap_alloc(size_t size) {
    static uint8_t fake_heap[512 * 1024] ALIGNED(4096);  // 512KB aligned heap
    static size_t heap_offset = 0;
    static bool heap_initialized = false;
    
    // Initialize heap on first use
    if (!heap_initialized) {
        memset(fake_heap, 0, sizeof(fake_heap));
        heap_initialized = true;
        GFX_LOG_HEX("Heap initialized: 512KB at ", (uint32_t)fake_heap);
    }
    
    // Validate size (align to 4 bytes)
    if (size == 0 || size > (256 * 1024)) {  // Max 256KB per allocation
        gfx_print("Invalid heap allocation size: ");
        gfx_print_hex(size);
        gfx_print("\n");
        return NULL;
    }
    
    // Align size to 4 bytes
    size = (size + 3) & ~3;
    
    if (heap_offset + size >= sizeof(fake_heap)) {
        gfx_print("Heap exhausted - requested: ");
        gfx_print_hex(size);
        gfx_print(" available: ");
        gfx_print_hex(sizeof(fake_heap) - heap_offset);
        gfx_print("\n");
        return NULL;
    }
    
    void* ptr = &fake_heap[heap_offset];
    heap_offset += size;
    
    // Clear allocated memory
    memset(ptr, 0, size);
    
    return ptr;
}

// Simple heap free function (stub implementation)
void heap_free(void* ptr) {
    // TODO: Implement proper heap free with linked list of free blocks
    // For now, just track that free was called
    if (ptr == NULL) {
        return;
    }
    
    static uint32_t free_count = 0;
    free_count++;
    
    // In a proper implementation, we would:
    // 1. Add the freed block to a free list
    // 2. Coalesce adjacent free blocks
    // 3. Update heap metadata
    
    // For debugging, just zero out the memory
    // (This is unsafe in a real system but helps catch use-after-free bugs)
    // Note: We don't know the size, so we can't safely do this
    
    gfx_print("heap_free() called (");
    gfx_print_decimal(free_count);
    gfx_print(" total frees) - ptr=");
    gfx_print_hex((uint32_t)ptr);
    gfx_print("\n");
}

// Simple delay function
void kernel_delay(uint32_t count) {
    for (volatile uint32_t i = 0; i < count; i++) {
        // Busy wait
    }
}

void* sbrk(int increment) {
    if (heap_end + increment > heap + HEAP_SIZE) return (void*)-1;
    void* prev = heap_end;
    heap_end += increment;
    return prev;
}