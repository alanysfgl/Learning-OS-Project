[GLOBAL tss_flush]

tss_flush:
    mov ax, 0x2B        ; TSS selector (index 5, RPL=3)
    ltr ax
    ret
