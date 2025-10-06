; Boot stub that bypasses multiboot validation to see what magic we get
BITS 32

section .multiboot
align 4
multiboot_header:
    dd 0x1BADB002              ; Magic number (multiboot spec)
    dd 0x00000003              ; Flags (align modules + memory info ONLY)
    dd -(0x1BADB002 + 0x00000003)  ; Checksum

section .text
global _start
extern kernel_main

_start:
    ; Set up stack
    mov esp, stack_top
    
    ; Always call kernel_main regardless of magic (for debugging)
    push ebx    ; multiboot_info 
    push eax    ; multiboot_magic (whatever it is)
    call kernel_main
    
    ; Clean up and halt
    add esp, 8
    cli
    hlt

section .bss
align 4
stack_bottom:
    resb 16384
stack_top: