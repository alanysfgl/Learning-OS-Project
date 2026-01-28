ENTRY(_start)
SECTIONS {
    . = 1M; /* Kernel 1MB adresinden başlasın */
    .text : { *(.multiboot) *(.text) }
    .data : { *(.data) }
    .bss  : { *(.bss) }
}
