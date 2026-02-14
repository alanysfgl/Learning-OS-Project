[GLOBAL gdt_flush]    ; C kodundan çağrılabilmesi için

gdt_flush:
    mov eax, [esp+4]  ; C'den gönderilen GDT adresini al
    lgdt [eax]        ; GDT'yi işlemciye yükle

    ; Segment registerlarını güncelle
    mov ax, 0x10      ; 0x10 GDT'deki data segmentine işaret eder
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.flush   ; 0x08 code segmentine uzak zıplama (Far Jump)
.flush:
    ret
section .note.GNU-stack noalloc noexec nowrite progbits
