# Derleyiciler ve bayraklar
CC = gcc
AS = nasm
LDFLAGS = -m elf_i386 -T linker.ld
ASFLAGS = -f elf32
CFLAGS = -m32 -c -std=gnu99 -ffreestanding -O2 -Wall -Wextra

# Dosyalar
SOURCES = boot.o kernel.o gdt.o gdt_asm.o idt.o idt_asm.o interrupt.o monitor.o
OUTPUT = mykernel.bin

all: $(OUTPUT)

$(OUTPUT): $(SOURCES)
	ld $(LDFLAGS) -o $(OUTPUT) $(SOURCES)

boot.o: boot.s
	$(AS) $(ASFLAGS) boot.s -o boot.o

gdt_asm.o: gdt_asm.s
	$(AS) $(ASFLAGS) gdt_asm.s -o gdt_asm.o

interrupt.o: interrupt.s
	$(AS) $(ASFLAGS) interrupt.s -o interrupt.o
monitor.o: monitor.c
	$(CC) $(CFLAGS) monitor.c -o monitor.o	

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o $(OUTPUT)

run: all
	qemu-system-i386 -kernel $(OUTPUT)