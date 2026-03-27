// pci.h
#ifndef PCI_H
#define PCI_H

#include "common.h"

u32int pci_read_dword(u8int bus, u8int slot, u8int func, u8int offset);
void pci_write_dword(u8int bus, u8int slot, u8int func, u8int offset, u32int value);
int pci_find_device(u16int vendor, u16int device, u8int *bus, u8int *slot, u8int *func);

#endif
