[BITS 32]

section .text

global irq_common_stub
extern interrupt_handler

global idt_flush
global irqdefault

; Export IRQ stubs
%assign i 32
%rep 16
    %define irq_label irq %+ i
    global irq_label
%assign i i+1
%endrep

; IRQ stub macro
%macro IRQ_STUB_NO_ERRCODE 1
    push byte 0
    push byte %1
    jmp irq_common_stub
%endmacro


irq_common_stub:
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

    mov eax, [esp + 20]     ; error code
    mov ebx, [esp + 24]     ; interrupt number

    push eax
    push ebx
    call interrupt_handler
    add esp, 8

    pop gs
    pop fs
    pop es
    pop ds
    popa

    add esp, 8              ; discard pushed error code + int number
    iret
; Generate IRQ stubs
%assign i 32
%rep 16
    %define irq_label irq %+ i
irq_label:
    IRQ_STUB_NO_ERRCODE i
%assign i i+1
%endrep

; Default handler
irqdefault:
    push byte 0
    push dword 255  ; Use dword to avoid signed byte overflow warning
    jmp irq_common_stub

; IDT flush
idt_flush:
    mov eax, [esp+4]
    lidt [eax]
    ret

section .data
global irq_stubs
irq_stubs:
    dd irq32, irq33, irq34, irq35
    dd irq36, irq37, irq38, irq39
    dd irq40, irq41, irq42, irq43
    dd irq44, irq45, irq46, irq47