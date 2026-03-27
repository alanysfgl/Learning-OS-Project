// ata.c
#include "ata.h"
#include "common.h"
#include "kprintf.h"
#include "storage.h"

#define ATA_PRIMARY_IO  0x1F0
#define ATA_PRIMARY_CTL 0x3F6

#define ATA_REG_DATA    0
#define ATA_REG_ERROR   1
#define ATA_REG_SECCNT0 2
#define ATA_REG_LBA0    3
#define ATA_REG_LBA1    4
#define ATA_REG_LBA2    5
#define ATA_REG_HDDEVSEL 6
#define ATA_REG_STATUS  7
#define ATA_REG_COMMAND 7

#define ATA_CMD_IDENTIFY 0xEC
#define ATA_CMD_READ_SECTORS 0x20
#define ATA_CMD_WRITE_SECTORS 0x30

#define ATA_SR_BSY 0x80
#define ATA_SR_DRQ 0x08
#define ATA_SR_ERR 0x01

static void ata_io_wait(void) {
    inb(ATA_PRIMARY_CTL);
    inb(ATA_PRIMARY_CTL);
    inb(ATA_PRIMARY_CTL);
    inb(ATA_PRIMARY_CTL);
}

struct ata_device {
    u16int io;
    u16int ctrl;
    u8int slave;
};

static int ata_wait_ready(struct ata_device *dev) {
    u32int timeout = 100000;
    while (timeout--) {
        u8int status = inb(dev->io + ATA_REG_STATUS);
        if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ)) {
            return 1;
        }
        if (status & ATA_SR_ERR) {
            return 0;
        }
    }
    return 0;
}

static int ata_read28(struct ata_device *dev, u32int lba, u8int count, void *buffer) {
    if (!dev || !buffer || count == 0) return 0;
    if (lba > 0x0FFFFFFF) return 0;

    u16int *buf = (u16int *)buffer;
    for (u8int c = 0; c < count; c++) {
        u32int cur = lba + c;
        int ok = 0;
        for (int attempt = 0; attempt < 3; attempt++) {
            outb(dev->io + ATA_REG_HDDEVSEL, 0xE0 | (dev->slave << 4) | ((cur >> 24) & 0x0F));
            ata_io_wait();
            outb(dev->io + ATA_REG_SECCNT0, 1);
            outb(dev->io + ATA_REG_LBA0, (u8int)(cur & 0xFF));
            outb(dev->io + ATA_REG_LBA1, (u8int)((cur >> 8) & 0xFF));
            outb(dev->io + ATA_REG_LBA2, (u8int)((cur >> 16) & 0xFF));
            outb(dev->io + ATA_REG_COMMAND, ATA_CMD_READ_SECTORS);

            if (!ata_wait_ready(dev)) {
                continue;
            }
            for (int i = 0; i < 256; i++) {
                *buf++ = inw(dev->io + ATA_REG_DATA);
            }
            ata_io_wait();
            ok = 1;
            break;
        }
        if (!ok) {
            kprintf("ATA read error at LBA %u\n", cur);
            return c;
        }
    }
    return count;
}

static int ata_write28(struct ata_device *dev, u32int lba, u8int count, const void *buffer) {
    if (!dev || !buffer || count == 0) return 0;
    if (lba > 0x0FFFFFFF) return 0;

    const u16int *buf = (const u16int *)buffer;
    for (u8int c = 0; c < count; c++) {
        u32int cur = lba + c;
        int ok = 0;
        for (int attempt = 0; attempt < 3; attempt++) {
            outb(dev->io + ATA_REG_HDDEVSEL, 0xE0 | (dev->slave << 4) | ((cur >> 24) & 0x0F));
            ata_io_wait();
            outb(dev->io + ATA_REG_SECCNT0, 1);
            outb(dev->io + ATA_REG_LBA0, (u8int)(cur & 0xFF));
            outb(dev->io + ATA_REG_LBA1, (u8int)((cur >> 8) & 0xFF));
            outb(dev->io + ATA_REG_LBA2, (u8int)((cur >> 16) & 0xFF));
            outb(dev->io + ATA_REG_COMMAND, ATA_CMD_WRITE_SECTORS);

            if (!ata_wait_ready(dev)) {
                continue;
            }
            for (int i = 0; i < 256; i++) {
                outw(dev->io + ATA_REG_DATA, *buf++);
            }
            ata_io_wait();
            ok = 1;
            break;
        }
        if (!ok) {
            kprintf("ATA write error at LBA %u\n", cur);
            return c;
        }
    }
    return count;
}

static int ata_storage_read(struct storage_device *dev, u32int lba, u8int count, void *buffer) {
    if (!dev || !dev->driver_data) return 0;
    return ata_read28((struct ata_device *)dev->driver_data, lba, count, buffer);
}

static int ata_storage_write(struct storage_device *dev, u32int lba, u8int count, const void *buffer) {
    if (!dev || !dev->driver_data) return 0;
    return ata_write28((struct ata_device *)dev->driver_data, lba, count, buffer);
}

static int ata_wait_busy(void) {
    u32int timeout = 100000;
    while (timeout--) {
        if (!(inb(ATA_PRIMARY_IO + ATA_REG_STATUS) & ATA_SR_BSY)) {
            return 1;
        }
    }
    return 0;
}

static int ata_wait_drq(void) {
    u32int timeout = 100000;
    while (timeout--) {
        u8int status = inb(ATA_PRIMARY_IO + ATA_REG_STATUS);
        if (status & ATA_SR_ERR) {
            return 0;
        }
        if (status & ATA_SR_DRQ) {
            return 1;
        }
    }
    return 0;
}

static int ata_identify_primary(struct ata_device *adev, struct storage_device *dev) {
    outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xA0); // master
    ata_io_wait();

    outb(ATA_PRIMARY_IO + ATA_REG_SECCNT0, 0);
    outb(ATA_PRIMARY_IO + ATA_REG_LBA0, 0);
    outb(ATA_PRIMARY_IO + ATA_REG_LBA1, 0);
    outb(ATA_PRIMARY_IO + ATA_REG_LBA2, 0);
    outb(ATA_PRIMARY_IO + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);

    if (inb(ATA_PRIMARY_IO + ATA_REG_STATUS) == 0) {
        return 0;
    }

    if (!ata_wait_busy()) return 0;
    if (!ata_wait_drq()) return 0;

    u16int data[256];
    for (int i = 0; i < 256; i++) {
        data[i] = inw(ATA_PRIMARY_IO + ATA_REG_DATA);
    }

    char model[41];
    for (int i = 0; i < 20; i++) {
        model[i * 2] = (char)(data[27 + i] >> 8);
        model[i * 2 + 1] = (char)(data[27 + i] & 0xFF);
    }
    model[40] = 0;

    if (dev) {
        dev->name = "ata0";
        dev->sector_size = 512;
        dev->sector_count = 0;
        dev->read = 0;
        dev->write = 0;
        dev->driver_data = adev;
        storage_register(dev);
    }

    kprintf("ATA PIO: Primary Master model: %s\n", model);
    return 1;
}

void ata_init(void) {
    static struct ata_device adev;
    static struct storage_device ata_dev;
    adev.io = ATA_PRIMARY_IO;
    adev.ctrl = ATA_PRIMARY_CTL;
    adev.slave = 0;

    if (!ata_identify_primary(&adev, &ata_dev)) {
        kprintf("ATA PIO: Primary Master not detected.\n");
        return;
    }

    ata_dev.read = ata_storage_read;
    ata_dev.write = ata_storage_write;
}
