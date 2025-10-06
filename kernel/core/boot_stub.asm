; QuantumOS Kernel Boot Stub
; Multiboot-compliant kernel entry point with smart video mode handling

; QuantumOS Kernel Boot Stub
; Multiboot-compliant kernel entry point

BITS 32

section .multiboot
align 4
multiboot_header:
    dd 0x1BADB002              ; magic
    dd 0x00000007              ; flags: mem + device + video
    dd -(0x1BADB002 + 0x00000007) ; checksum

    dd 1                       ; mode type: graphics
    dd 1024                    ; width
    dd 768                     ; height
    dd 32                      ; depth
    
section .text
global _start
extern kernel_main
align 4
_start:


    mov [multiboot_magic], eax
    mov [multiboot_info], ebx

    mov esp, stack_bottom
    add esp, 16384

    lgdt [gdt_descriptor]
letsjmp:
    jmp 0x08:.reload_segments
.reload_segments:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    cmp dword [multiboot_magic], 0x2BADB002
    jne .bad_multiboot

    push dword [multiboot_info]
    push dword [multiboot_magic]
    call kernel_main
    add esp, 8

    jmp .halt

.bad_multiboot:
    jmp .halt

.halt:
    cli
hang:
    hlt
    jmp hang

section .data
; Basic GDT for early boot
align 8
global gdt_descriptor
global gdt_start
gdt_start:
    ; Null descriptor (required)
    dd 0x00000000
    dd 0x00000000
    
    ; Kernel code segment (0x08)
    ; Base=0, Limit=4GB, Access=0x9A (present, ring 0, executable, readable)
    ; Flags=0xCF (4KB granularity, 32-bit)
    dw 0xFFFF       ; Limit 0-15q
    dw 0x0000       ; Base 0-15
    db 0x00         ; Base 16-23
    db 0x9A         ; Access byte (present, ring 0, executable, readable)
    db 0xCF         ; Flags + Limit 16-19 (4KB granularity, 32-bit)
    db 0x00         ; Base 24-31
    
    ; Kernel data segment (0x10)
    ; Base=0, Limit=4GB, Access=0x92 (present, ring 0, writable)
    ; Flags=0xCF (4KB granularity, 32-bit)
    dw 0xFFFF       ; Limit 0-15
    dw 0x0000       ; Base 0-15
    db 0x00         ; Base 16-23
    db 0x92         ; Access byte (present, ring 0, writable)
    db 0xCF         ; Flags + Limit 16-19
    db 0x00         ; Base 24-31
gdt_end:

; GDT descriptor
gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; GDT size
    dd gdt_start                ; GDT address

section .bss
align 4
stack_bottom:
    resb 16384  ; 16 KB stack
stack_top:

; Storage for multiboot information
multiboot_magic: resd 1
multiboot_info:  resd 1