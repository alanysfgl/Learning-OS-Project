[GLOBAL task_switch]
[GLOBAL task_start]

task_switch:
    mov eax, [esp+4]    ; old
    mov edx, [esp+8]    ; new

    mov [eax+0], esp
    mov [eax+4], ebp
    mov [eax+8], ebx
    mov [eax+12], esi
    mov [eax+16], edi

    call .get_eip
.get_eip:
    pop ecx
    mov [eax+20], ecx

    mov esp, [edx+0]
    mov ebp, [edx+4]
    mov ebx, [edx+8]
    mov esi, [edx+12]
    mov edi, [edx+16]
    mov ecx, [edx+20]
    jmp ecx

; Start a task by restoring an interrupt frame and iret into it.
; void task_start(u32int new_esp)
task_start:
    mov eax, [esp+4]
    mov esp, eax

    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa
    add esp, 8
    iret
