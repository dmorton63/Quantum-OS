; QuantumOS - GDT Assembly Functions
; Assembly functions for loading GDT and setting up segments

[BITS 32]

global gdt_flush

; gdt_flush - Load new GDT and update segment registers
; Parameter: address of GDT pointer structure
gdt_flush:
    mov eax, [esp+4]    ; Get GDT pointer from stack
    lgdt [eax]          ; Load new GDT
    
    ; Update segment registers
    mov ax, 0x10        ; Kernel data segment selector (GDT entry 2)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Far jump to update CS register with kernel code segment
    jmp 0x08:.flush    ; Kernel code segment selector (GDT entry 1)
    
.flush:
    ret