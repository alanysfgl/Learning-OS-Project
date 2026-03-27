# Derleyiciler ve bayraklar
CC = gcc
AS = nasm
LD = ld

SRC_DIR = src
BOOT_DIR = src/boot
CORE_DIR = src/core
INT_DIR = src/interrupt
MM_DIR = src/mm
PROC_DIR = src/proc
FS_DIR = src/fs
NET_DIR = src/net
USER_DIR = src/user
LIB_DIR = src/lib
DRV_DIR = src/drivers
ASM_DIR = asm
INC_DIR = include
BUILD_DIR = build
LINKER_DIR = linker

LDFLAGS = -m elf_i386 -T $(LINKER_DIR)/linker.ld
ASFLAGS = -f elf32
CFLAGS = -m32 -c -std=gnu99 -ffreestanding -O2 -Wall -Wextra -I$(INC_DIR)

# Dosyalar
OBJS = \
	$(BUILD_DIR)/boot.o \
	$(BUILD_DIR)/kernel.o \
	$(BUILD_DIR)/gdt.o \
	$(BUILD_DIR)/gdt_asm.o \
	$(BUILD_DIR)/usermode.o \
	$(BUILD_DIR)/tss.o \
	$(BUILD_DIR)/task_switch.o \
	$(BUILD_DIR)/idt.o \
	$(BUILD_DIR)/idt_asm.o \
	$(BUILD_DIR)/interrupt.o \
	$(BUILD_DIR)/monitor.o \
	$(BUILD_DIR)/kprintf.o \
	$(BUILD_DIR)/libc.o \
	$(BUILD_DIR)/panic.o \
	$(BUILD_DIR)/logger.o \
	$(BUILD_DIR)/input.o \
	$(BUILD_DIR)/userlib.o \
	$(BUILD_DIR)/init.o \
	$(BUILD_DIR)/user_shell.o \
	$(BUILD_DIR)/ipc.o \
	$(BUILD_DIR)/pci.o \
	$(BUILD_DIR)/rtl8139.o \
	$(BUILD_DIR)/smp.o \
	$(BUILD_DIR)/multiboot.o \
	$(BUILD_DIR)/memory.o \
	$(BUILD_DIR)/paging.o \
	$(BUILD_DIR)/heap.o \
	$(BUILD_DIR)/scheduler.o \
	$(BUILD_DIR)/task.o \
	$(BUILD_DIR)/syscall.o \
	$(BUILD_DIR)/irq.o \
	$(BUILD_DIR)/mouse.o \
	$(BUILD_DIR)/serial.o \
	$(BUILD_DIR)/ata.o \
	$(BUILD_DIR)/storage.o \
	$(BUILD_DIR)/vfs.o \
	$(BUILD_DIR)/ramfs.o \
	$(BUILD_DIR)/fat.o \
	$(BUILD_DIR)/elf.o \
	$(BUILD_DIR)/shell.o \
	$(BUILD_DIR)/keyboard.o \
	$(BUILD_DIR)/keyboard_buffer.o \
	$(BUILD_DIR)/timer.o \
	$(BUILD_DIR)/pic.o \
	$(BUILD_DIR)/pit.o

OUTPUT = $(BUILD_DIR)/mykernel.bin

all: $(OUTPUT)

$(OUTPUT): $(OBJS)
	$(LD) $(LDFLAGS) -o $(OUTPUT) $(OBJS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/boot.o: $(ASM_DIR)/boot.s | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/gdt_asm.o: $(ASM_DIR)/gdt_asm.s | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/usermode.o: $(ASM_DIR)/usermode.s | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/tss.o: $(ASM_DIR)/tss.s | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/task_switch.o: $(ASM_DIR)/task_switch.s | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/idt_asm.o: $(ASM_DIR)/idt_asm.s | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/interrupt.o: $(ASM_DIR)/interrupt.s | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/kernel.o: $(CORE_DIR)/kernel.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/gdt.o: $(INT_DIR)/gdt.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/idt.o: $(INT_DIR)/idt.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/monitor.o: $(LIB_DIR)/monitor.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/kprintf.o: $(LIB_DIR)/kprintf.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/libc.o: $(LIB_DIR)/libc.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/panic.o: $(LIB_DIR)/panic.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/logger.o: $(LIB_DIR)/logger.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/input.o: $(LIB_DIR)/input.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/userlib.o: $(USER_DIR)/userlib.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/init.o: $(CORE_DIR)/init.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/user_shell.o: $(USER_DIR)/user_shell.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/ipc.o: $(PROC_DIR)/ipc.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/pci.o: $(NET_DIR)/pci.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/rtl8139.o: $(NET_DIR)/rtl8139.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/smp.o: $(BOOT_DIR)/smp.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/multiboot.o: $(BOOT_DIR)/multiboot.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/memory.o: $(MM_DIR)/memory.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/paging.o: $(MM_DIR)/paging.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/heap.o: $(MM_DIR)/heap.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/task.o: $(PROC_DIR)/task.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/syscall.o: $(PROC_DIR)/syscall.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/scheduler.o: $(PROC_DIR)/scheduler.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/irq.o: $(INT_DIR)/irq.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/mouse.o: $(DRV_DIR)/mouse.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/serial.o: $(DRV_DIR)/serial.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/ata.o: $(DRV_DIR)/ata.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/storage.o: $(FS_DIR)/storage.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/vfs.o: $(FS_DIR)/vfs.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/ramfs.o: $(FS_DIR)/ramfs.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/fat.o: $(FS_DIR)/fat.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/elf.o: $(USER_DIR)/elf.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/shell.o: $(USER_DIR)/shell.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/keyboard.o: $(DRV_DIR)/keyboard.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/keyboard_buffer.o: $(DRV_DIR)/keyboard_buffer.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/timer.o: $(DRV_DIR)/timer.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/pic.o: $(DRV_DIR)/pic.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/pit.o: $(DRV_DIR)/pit.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(BUILD_DIR)/*.o $(OUTPUT)

run: all
	qemu-system-i386 -kernel $(OUTPUT)

run-headless: all
	qemu-system-i386 -display none -serial stdio -monitor none -kernel $(OUTPUT)

test: all
	./scripts/test_qemu.sh
