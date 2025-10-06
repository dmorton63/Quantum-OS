; QuantumOS Bootloader
; A revolutionary bootloader for quantum computing enabled OS

BITS 16
ORG 0x7c00

section .text
    global _start

_start:
    ; Clear interrupts and set up segments
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00

    ; Print QuantumOS boot message
    mov si, boot_msg
    call print_string

    ; Set up basic video mode
    mov ah, 0x00
    mov al, 0x03    ; 80x25 text mode
    int 0x10

    ; Enable A20 line for extended memory access
    call enable_a20

    ; Load Global Descriptor Table for protected mode
    lgdt [gdt_descriptor]

    ; Switch to protected mode
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    ; Jump to protected mode code
    jmp CODE_SEG:protected_mode

enable_a20:
    ; Enable A20 line through keyboard controller
    in al, 0x92
    or al, 2
    out 0x92, al
    ret

print_string:
    ; Print null-terminated string pointed to by SI
    lodsb
    or al, al
    jz .done
    mov ah, 0x0e
    int 0x10
    jmp print_string
.done:
    ret

BITS 32
protected_mode:
    ; Set up protected mode segments
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    ; Print protected mode message
    mov esi, protected_msg
    call print_string_32

    ; Initialize quantum subsystems
    call init_quantum_hardware

    ; Load and jump to kernel
    call load_kernel
    
    ; Jump to kernel entry point
    jmp 0x100000

init_quantum_hardware:
    ; Placeholder for quantum hardware initialization
    ; In real implementation, this would detect and configure quantum processors
    ret

load_kernel:
    ; Load kernel from disk (simplified - would use proper filesystem)
    ; For now, assume kernel is loaded at 0x100000
    mov esi, kernel_loaded_msg
    call print_string_32
    ret

print_string_32:
    ; Print string in protected mode
    mov edi, 0xb8000    ; VGA text buffer
    mov ah, 0x07        ; White on black
.loop:
    lodsb
    or al, al
    jz .done
    stosw
    jmp .loop
.done:
    ret

; Global Descriptor Table
section .data
gdt_start:
    ; Null descriptor
    dd 0x0
    dd 0x0

gdt_code:
    ; Code segment descriptor
    dw 0xffff       ; Limit low
    dw 0x0000       ; Base low
    db 0x00         ; Base middle
    db 10011010b    ; Access byte
    db 11001111b    ; Granularity
    db 0x00         ; Base high

gdt_data:
    ; Data segment descriptor
    dw 0xffff       ; Limit low
    dw 0x0000       ; Base low
    db 0x00         ; Base middle
    db 10010010b    ; Access byte
    db 11001111b    ; Granularity
    db 0x00         ; Base high

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; Size
    dd gdt_start                ; Offset

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

boot_msg db 'QuantumOS Bootloader v1.0', 13, 10
         db 'Initializing quantum computing subsystems...', 13, 10, 0

protected_msg db 'Protected mode active. Quantum cores online.', 0
kernel_loaded_msg db 'Kernel loaded. Transferring control...', 0

; Boot signature
times 510-($-$$) db 0
dw 0xaa55