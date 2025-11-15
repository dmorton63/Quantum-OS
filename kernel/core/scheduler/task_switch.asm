; task_switch.asm - Low-level context switching for i686
; This file provides assembly functions for saving and restoring CPU context
; during task switches in the Quantum OS kernel.

[BITS 32]

SECTION .text

; External symbols
extern task_mgr_current_task

; Exported symbols
global task_switch_context_asm
global task_save_context
global task_restore_context

; CPU context structure offsets (must match cpu_context_t in task_manager.h)
%define CTX_EAX     0
%define CTX_EBX     4  
%define CTX_ECX     8
%define CTX_EDX     12
%define CTX_ESI     16
%define CTX_EDI     20
%define CTX_EBP     24
%define CTX_ESP     28
%define CTX_EIP     32
%define CTX_EFLAGS  36
%define CTX_CS      40
%define CTX_DS      44
%define CTX_ES      48
%define CTX_FS      52
%define CTX_GS      56
%define CTX_SS      60

; Task structure offsets (must match task_t in task_manager.h)
; task_id(4) + name(32) + state(4) + priority(4) + flags(4) = 48 bytes
%define TASK_CONTEXT_OFFSET 48  ; Offset of context field in task_t

;
; task_switch_context_asm(task_t *from_task, task_t *to_task)
; Perform complete context switch between tasks
;
; Parameters:
;   [esp+4] = from_task (can be NULL for first task)
;   [esp+8] = to_task
;
task_switch_context_asm:
    ; Disable interrupts during context switch
    cli
    
    ; Save current registers
    push eax
    push ebx
    push ecx
    push edx
    push esi
    push edi
    push ebp
    
    ; Get parameters
    mov eax, [esp + 32]     ; from_task (esp+4 + 7*4 pushed registers)
    mov ebx, [esp + 36]     ; to_task   (esp+8 + 7*4 pushed registers)
    
    ; Test if from_task is NULL (first task switch)
    test eax, eax
    jz .load_new_task
    
    ; Save current context to from_task
    add eax, TASK_CONTEXT_OFFSET    ; Point to context field
    
    ; Save general registers (already on stack, pop and save)
    pop dword [eax + CTX_EBP]
    pop dword [eax + CTX_EDI]
    pop dword [eax + CTX_ESI]
    pop dword [eax + CTX_EDX]
    pop dword [eax + CTX_ECX]
    pop dword [eax + CTX_EBX]
    pop dword [eax + CTX_EAX]
    
    ; Save stack pointer (after our function call)
    mov [eax + CTX_ESP], esp
    
    ; Save instruction pointer (return address)
    mov ecx, [esp]
    mov [eax + CTX_EIP], ecx
    
    ; Save flags
    pushf
    pop dword [eax + CTX_EFLAGS]
    
    ; Save segment registers
    mov [eax + CTX_CS], cs
    mov [eax + CTX_DS], ds
    mov [eax + CTX_ES], es
    mov [eax + CTX_FS], fs
    mov [eax + CTX_GS], gs
    mov [eax + CTX_SS], ss
    
    jmp .load_new_task

.no_save:
    ; Clean up stack if we didn't save
    add esp, 28  ; 7 registers * 4 bytes
    
.load_new_task:
    ; Load context from to_task
    add ebx, TASK_CONTEXT_OFFSET    ; Point to context field
    
    ; Load segment registers first
    mov ax, [ebx + CTX_DS]
    mov ds, ax
    mov ax, [ebx + CTX_ES]
    mov es, ax
    mov ax, [ebx + CTX_FS]
    mov fs, ax
    mov ax, [ebx + CTX_GS]
    mov gs, ax
    mov ax, [ebx + CTX_SS]
    mov ss, ax
    
    ; Load stack pointer
    mov esp, [ebx + CTX_ESP]
    
    ; Load flags
    push dword [ebx + CTX_EFLAGS]
    popf
    
    ; Load general registers
    mov eax, [ebx + CTX_EAX]
    mov ecx, [ebx + CTX_ECX]
    mov edx, [ebx + CTX_EDX]
    mov esi, [ebx + CTX_ESI]
    mov edi, [ebx + CTX_EDI]
    mov ebp, [ebx + CTX_EBP]
    
    ; Load EBX last since we're using it
    push dword [ebx + CTX_EIP]      ; Push return address
    mov ebx, [ebx + CTX_EBX]
    
    ; Enable interrupts and jump to new task
    sti
    ret         ; Jump to EIP that we pushed

;
; task_save_context(cpu_context_t *context)
; Save current CPU state to context structure
;
; Parameters:
;   [esp+4] = context pointer
;
task_save_context:
    mov eax, [esp + 4]      ; Get context pointer
    
    ; Save general registers
    mov [eax + CTX_EAX], eax
    mov [eax + CTX_EBX], ebx
    mov [eax + CTX_ECX], ecx
    mov [eax + CTX_EDX], edx
    mov [eax + CTX_ESI], esi
    mov [eax + CTX_EDI], edi
    mov [eax + CTX_EBP], ebp
    mov [eax + CTX_ESP], esp
    
    ; Save return address as EIP
    mov ebx, [esp]
    mov [eax + CTX_EIP], ebx
    
    ; Save flags
    pushf
    pop dword [eax + CTX_EFLAGS]
    
    ; Save segment registers
    mov [eax + CTX_CS], cs
    mov [eax + CTX_DS], ds
    mov [eax + CTX_ES], es
    mov [eax + CTX_FS], fs
    mov [eax + CTX_GS], gs
    mov [eax + CTX_SS], ss
    
    ret

;
; task_restore_context(cpu_context_t *context)
; Restore CPU state from context structure
; Note: This function does not return to caller!
;
; Parameters:
;   [esp+4] = context pointer
;
task_restore_context:
    mov eax, [esp + 4]      ; Get context pointer
    
    ; Load segment registers
    mov bx, [eax + CTX_DS]
    mov ds, bx
    mov bx, [eax + CTX_ES]
    mov es, bx
    mov bx, [eax + CTX_FS]
    mov fs, bx
    mov bx, [eax + CTX_GS]
    mov gs, bx
    mov bx, [eax + CTX_SS]
    mov ss, bx
    
    ; Load stack pointer
    mov esp, [eax + CTX_ESP]
    
    ; Load flags
    push dword [eax + CTX_EFLAGS]
    popf
    
    ; Push return address for final jump
    push dword [eax + CTX_EIP]
    
    ; Load general registers (EAX last since we're using it)
    mov ebx, [eax + CTX_EBX]
    mov ecx, [eax + CTX_ECX]
    mov edx, [eax + CTX_EDX]
    mov esi, [eax + CTX_ESI]
    mov edi, [eax + CTX_EDI]
    mov ebp, [eax + CTX_EBP]
    mov eax, [eax + CTX_EAX]
    
    ; Jump to restored context
    ret

SECTION .data
    ; Reserved for future use

SECTION .bss
    ; Reserved for future use