[GLOBAL idt_flush]

idt_flush:
    mov eax, [esp+4]  ; IDT adresi
    lidt [eax]        ; IDT'yi yükle
    ret