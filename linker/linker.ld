ENTRY(_start)
SECTIONS {
    . = 1M; /* Kernel 1MB adresinden başlasın */
    __kernel_start = .;
    .text : { *(.multiboot) *(.text) }
    .data : { *(.data) }
    .bss  : { *(.bss) }
    __kernel_end = .;
}
