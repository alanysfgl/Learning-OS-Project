# 🚀 Learning-OS-Project

Learning-OS-Project is a lightweight **x86 (i386)** bare-metal kernel project focused on learning low-level OS fundamentals step by step.

## 🎯 Core Objectives

The primary goal of this kernel is to provide hands-on experience with:

1. **Low-Level Initialization:** Understanding how CPU execution starts and how protected mode setup works.
2. **Interrupt Handling:** Managing exceptions and hardware interrupts via GDT/IDT.
3. **Resource Constraints:** Writing simple, explicit, and efficient code for constrained environments.

## ✅ Current Features

- GDT initialization
- IDT initialization
- Basic exception handler (divide-by-zero)
- Keyboard IRQ handling
- VGA text-mode output with cursor update and scroll

## 🛠️ Prerequisites

To build and run this kernel:

- **Compiler:** `gcc` with 32-bit support (`-m32`)
- **Assembler:** `nasm`
- **Linker:** `ld` (i386 ELF support)
- **Build System:** `make`
- **Emulator:** `qemu-system-i386` (optional but recommended for `make run`)

> Note: This project currently targets **x86/i386**, not ARM.

## 🚀 Build and Run

```bash
# Clone repository
git clone https://github.com/alanysfgl/Learning-OS-Project.git
cd Learning-OS-Project/my_kernel

# Build kernel binary
make all

# Run with QEMU
make run
```

## 🗺️ Short Roadmap

1. Expand exception coverage (ISR 0–31)
2. Add PIT timer tick handling (IRQ0)
3. Improve keyboard handling (shift/caps/backspace behavior)
4. Start memory management (paging + basic kmalloc)
5. Add a tiny command shell (`help`, `clear`, `ticks`, `meminfo`)
