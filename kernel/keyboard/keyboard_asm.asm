[BITS 32]
section .text
extern keyboard_service_handler
global irqkeyboard
irqkeyboard:

    cli
    pusha
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax


    push esp
    call keyboard_service_handler

    add esp, 4
    pop gs
    pop fs
    pop es
    pop ds
    popa
    sti
    iret


