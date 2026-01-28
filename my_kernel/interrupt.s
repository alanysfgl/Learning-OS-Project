[EXTERN irq_handler]

%macro ISR_NOERRCODE 1
  [GLOBAL isr%1]
  isr%1:
    cli
    push byte 0
    push byte %1
    jmp common_stub
%endmacro

%macro IRQ 2
  [GLOBAL irq%1]
  irq%1:
    cli
    push byte 0
    push byte %2
    jmp common_stub
%endmacro

ISR_NOERRCODE 0
IRQ 1, 33

common_stub:
    pusha           ; Registerları sakla
    
    mov ax, ds      ; Mevcut data segmentini sakla
    push eax        ; EAX üzerinden yığına at (32 bit)

    mov ax, 0x10    ; Kernel data segmentini yükle
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call irq_handler

    pop eax         ; Orijinal data segmentini geri al
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa            ; Tüm genel registerları geri yükle
    add esp, 8      ; Hata kodu ve int no temizle
    sti
    iret            ; Kesmeden dön