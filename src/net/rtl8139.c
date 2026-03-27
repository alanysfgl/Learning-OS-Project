// rtl8139.c
#include "rtl8139.h"
#include "pci.h"
#include "memory.h"
#include "logger.h"
#include "libc.h"

#define RTL_VENDOR_ID 0x10EC
#define RTL_DEVICE_ID 0x8139

#define RTL_REG_CMD     0x37
#define RTL_REG_IMR     0x3C
#define RTL_REG_ISR     0x3E
#define RTL_REG_RBSTART 0x30
#define RTL_REG_RCR     0x44
#define RTL_REG_TCR     0x40
#define RTL_REG_TXADDR0 0x20
#define RTL_REG_TXSTAT0 0x10

static u16int rtl_io = 0;
static u8int *rx_buffer = 0;
static u8int *tx_buffer = 0;
static u32int tx_buf_phys = 0;

static void rtl_outb(u16int reg, u8int val) { outb(rtl_io + reg, val); }
static void rtl_outw(u16int reg, u16int val) { outw(rtl_io + reg, val); }
static void rtl_outl(u16int reg, u32int val) { outl(rtl_io + reg, val); }
static u8int rtl_inb(u16int reg) { return inb(rtl_io + reg); }

int rtl8139_init(void) {
    u8int bus, slot, func;
    if (!pci_find_device(RTL_VENDOR_ID, RTL_DEVICE_ID, &bus, &slot, &func)) {
        log_warn("RTL8139: device not found.\n");
        return -1;
    }

    u32int bar0 = pci_read_dword(bus, slot, func, 0x10);
    if (!(bar0 & 0x1)) {
        log_warn("RTL8139: BAR0 is not I/O.\n");
        return -1;
    }
    rtl_io = (u16int)(bar0 & ~0x3);

    u32int cmd = pci_read_dword(bus, slot, func, 0x04);
    cmd |= (1 << 2) | (1 << 0); // bus master + IO space
    pci_write_dword(bus, slot, func, 0x04, cmd);

    u32int rx_phys = frame_alloc_contiguous(3);
    if (!rx_phys) return -1;
    rx_buffer = (u8int *)rx_phys;

    tx_buf_phys = frame_alloc_contiguous(1);
    if (!tx_buf_phys) return -1;
    tx_buffer = (u8int *)tx_buf_phys;

    rtl_outb(RTL_REG_CMD, 0x10); // reset
    while (rtl_inb(RTL_REG_CMD) & 0x10) { }

    rtl_outl(RTL_REG_RBSTART, rx_phys);
    rtl_outl(RTL_REG_RCR, 0x0000000F); // AB+AM+APM+AAP
    rtl_outl(RTL_REG_TCR, 0x00000000);
    rtl_outw(RTL_REG_IMR, 0x0000);
    rtl_outw(RTL_REG_ISR, 0xFFFF);

    rtl_outb(RTL_REG_CMD, 0x0C); // RX/TX enable
    log_info("RTL8139: init ok at io=0x%x\n", rtl_io);
    return 0;
}

int rtl8139_send(const void *data, u32int len) {
    if (!rtl_io || !tx_buffer || !data || len == 0) return -1;
    if (len > 1500) len = 1500;
    memcpy(tx_buffer, data, len);
    rtl_outl(RTL_REG_TXADDR0, tx_buf_phys);
    rtl_outl(RTL_REG_TXSTAT0, len);
    return 0;
}
