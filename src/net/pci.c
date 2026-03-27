// pci.c
#include "pci.h"

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

u32int pci_read_dword(u8int bus, u8int slot, u8int func, u8int offset) {
    u32int address = (1U << 31) |
                     ((u32int)bus << 16) |
                     ((u32int)slot << 11) |
                     ((u32int)func << 8) |
                     (offset & 0xFC);
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

void pci_write_dword(u8int bus, u8int slot, u8int func, u8int offset, u32int value) {
    u32int address = (1U << 31) |
                     ((u32int)bus << 16) |
                     ((u32int)slot << 11) |
                     ((u32int)func << 8) |
                     (offset & 0xFC);
    outl(PCI_CONFIG_ADDRESS, address);
    outl(PCI_CONFIG_DATA, value);
}

int pci_find_device(u16int vendor, u16int device, u8int *bus_out, u8int *slot_out, u8int *func_out) {
    for (u16int bus = 0; bus < 256; bus++) {
        for (u8int slot = 0; slot < 32; slot++) {
            for (u8int func = 0; func < 8; func++) {
                u32int val = pci_read_dword((u8int)bus, slot, func, 0x00);
                if ((val & 0xFFFF) == 0xFFFF) {
                    if (func == 0) break;
                    continue;
                }
                u16int ven = (u16int)(val & 0xFFFF);
                u16int dev = (u16int)((val >> 16) & 0xFFFF);
                if (ven == vendor && dev == device) {
                    if (bus_out) *bus_out = (u8int)bus;
                    if (slot_out) *slot_out = slot;
                    if (func_out) *func_out = func;
                    return 1;
                }
            }
        }
    }
    return 0;
}
