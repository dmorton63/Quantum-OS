[BITS 32]
section .text
extern mouse_handler
global irqmouse
irqmouse:
    cli
    pushad
    call mouse_handler
    popad
    sti
    iret

