[GLOBAL enter_user_mode]

; void enter_user_mode(u32int entry, u32int user_stack)
enter_user_mode:
    cli
    mov eax, [esp+8]    ; user_stack
    mov ebx, [esp+4]    ; entry

    mov ax, 0x23        ; user data segment (RPL=3)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push 0x23           ; SS
    push eax            ; ESP
    pushfd
    pop eax
    or eax, 0x200       ; IF
    push eax            ; EFLAGS
    push 0x1B           ; CS (user code, RPL=3)
    push ebx            ; EIP
    iret
