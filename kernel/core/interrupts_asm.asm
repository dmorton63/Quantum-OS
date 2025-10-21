[BITS 32]
section .text
extern interrupt_handler

; ────────────────
; ISR Stubs (Exceptions 0–31)
; ────────────────
%macro ISR_STUB_NO_ERRCODE 1
global isr%1
isr%1:
    push dword 0         ; Fake error code
    push dword %1        ; Interrupt number
    jmp isr_common_stub
%endmacro

%macro ISR_STUB_ERRCODE 1
global isr%1
isr%1:
    push dword %1        ; Interrupt number
    jmp isr_common_stub
%endmacro

%assign i 0
%rep 32
    ISR_STUB_NO_ERRCODE i
%assign i i+1
%endrep

; ────────────────
; IRQ Stubs (IRQs 0–15 → Vectors 32–47)
; ────────────────
%macro IRQ_STUB 1
%if %1 = 33
extern irqkeyboard
global irq33
irq33:
    jmp irqkeyboard
%elif %1 = 44
extern irqmouse
global irq44
irq44:
    jmp irqmouse
%else
    global irq%1
irq%1:
    push dword 0         ; Error code
    push dword %1        ; Interrupt number
    jmp irq_common_stub
%endif
%endmacro

%assign i 32
%rep 16
    IRQ_STUB i
%assign i i+1
%endrep

; ────────────────
; Dedicated IRQ1 Stub (Keyboard)
; ────────────────

extern timer_handler
global irq0_handler
irq0_handler:
    push dword 0          ; dummy error code
    push dword 32         ; vector number
    call timer_handler
    add esp, 8
    iret

; ────────────────
; Default IRQ Stub (Fallback)
; ────────────────
global irqdefault
irqdefault:
    push dword 0
    push dword 255       ; Unknown IRQ
    jmp irq_common_stub

; ────────────────
; ISR Common Stub
; ────────────────
isr_common_stub:
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

    mov ebx, [esp + 52]  ; error code
    mov eax, [esp + 56]  ; interrupt number

    push eax
    push ebx
    call interrupt_handler
    add esp, 8

    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8
    iret

; ────────────────
; IRQ Common Stub
; ────────────────
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

    mov ebx, [esp + 52]  ; error code
    mov eax, [esp + 56]  ; interrupt number

    push eax
    push ebx
    call interrupt_handler
    add esp, 8

    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8
    iret

; ────────────────
; IDT Flush
; ────────────────
global idt_flush
idt_flush:
    mov eax, [esp + 4]
    lidt [eax]
    ret
section .data
global irq_stubs
irq_stubs:
    dd irq32, irq33, irq34, irq35
    dd irq36, irq37, irq38, irq39
    dd irq40, irq41, irq42, irq43
    dd irq44, irq45, irq46, irq47

section .data
global isr_stubs
isr_stubs:
    dd isr0, isr1, isr2, isr3
    dd isr4, isr5, isr6, isr7
    dd isr8, isr9, isr10, isr11
    dd isr12, isr13, isr14, isr15
    dd isr16, isr17, isr18, isr19
    dd isr20, isr21, isr22, isr23
    dd isr24, isr25, isr26, isr27
    dd isr28, isr29, isr30, isr31