# 🚀 Learning-OS-Project

Learning-OS-Project is a lightweight, bare-metal kernel project developed to explore the fundamental principles of operating system architecture and embedded systems engineering.

## 🎯 Core Objectives
The primary goal of this kernel is to provide hands-on experience with:
1. **Low-Level Initialization:** Understanding how a CPU begins execution from power-on.
2. **Interrupt Handling:** Managing hardware interrupts at the software level.
3. **Resource Constraints:** Optimizing code for memory and performance-limited environments.

## 🚀 Getting Started

### Prerequisites
To build and run this kernel, you will need the following tools:
* **Toolchain:** `gcc-arm-none-eabi` (or your specific cross-compiler)
* **Simulator:** `QEMU` (or physical hardware: [Your Board Model])
* **Build System:** `Make`

### Build and Run
```bash
# Clone the repository
git clone [https://github.com/yourusername/your-project-name.git](https://github.com/yourusername/your-project-name.git)

# Navigate to the project directory
cd your-project-name

# Compile the project
make all

# Run using QEMU
make run
