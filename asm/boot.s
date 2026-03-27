; Multiboot header
section .multiboot
    align 4
    dd 0x1BADB002              ; Magic number
    dd 0x00                    ; Flags
    dd -(0x1BADB002 + 0x00)    ; Checksum

section .bss
align 16
stack_bottom:
    resb 16384                 ; 16 KB Stack alanı ayır
stack_top:

section .text
global _start
extern kernel_main

_start:
    mov esp, stack_top         ; İşlemciye yığının nerede olduğunu söyle (KRİTİK!)

    push ebx                   ; multiboot info addr
    push eax                   ; multiboot magic
    call kernel_main           ; C koduna atla
    add esp, 8

.hang:                         ; Kernel'dan dönülürse sonsuz döngüye gir
    hlt
    jmp .hang
