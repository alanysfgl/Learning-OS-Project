// mouse.c
#include "mouse.h"
#include "common.h"
#include "irq.h"
#include "kprintf.h"
#include "input.h"

#define MOUSE_DATA_PORT 0x60
#define MOUSE_STATUS_PORT 0x64
#define MOUSE_COMMAND_PORT 0x64

#define MOUSE_CMD_ENABLE_AUX 0xA8
#define MOUSE_CMD_READ_CTR  0x20
#define MOUSE_CMD_WRITE_CTR 0x60

#define MOUSE_DEV_SET_DEFAULTS 0xF6
#define MOUSE_DEV_ENABLE       0xF4

static u8int mouse_cycle = 0;
static s8int mouse_bytes[3];
static volatile u32int packet_count = 0;

static void mouse_wait(u8int type) {
    u32int timeout = 100000;
    if (type == 0) {
        while (timeout--) {
            if (inb(MOUSE_STATUS_PORT) & 1) return;
        }
    } else {
        while (timeout--) {
            if (!(inb(MOUSE_STATUS_PORT) & 2)) return;
        }
    }
}

static void mouse_write(u8int data) {
    mouse_wait(1);
    outb(MOUSE_COMMAND_PORT, 0xD4);
    mouse_wait(1);
    outb(MOUSE_DATA_PORT, data);
}

static u8int mouse_read(void) {
    mouse_wait(0);
    return inb(MOUSE_DATA_PORT);
}

static void mouse_handler(struct registers *regs) {
    (void)regs;
    u8int status = inb(MOUSE_STATUS_PORT);
    if (!(status & 1)) return;

    s8int data = (s8int)inb(MOUSE_DATA_PORT);
    switch (mouse_cycle) {
        case 0:
            if (!(data & 0x08)) {
                return;
            }
            mouse_bytes[0] = data;
            mouse_cycle = 1;
            break;
        case 1:
            mouse_bytes[1] = data;
            mouse_cycle = 2;
            break;
        case 2:
            mouse_bytes[2] = data;
            mouse_cycle = 0;
            packet_count++;
            input_push_mouse(mouse_bytes[1], mouse_bytes[2], (u8int)(mouse_bytes[0] & 0x07));
            if ((packet_count % 50) == 0) {
                kprintf("Mouse packet: b=%d dx=%d dy=%d\n",
                        (int)mouse_bytes[0], (int)mouse_bytes[1], (int)mouse_bytes[2]);
            }
            break;
    }
}

void mouse_init(void) {
    // Enable auxiliary device
    mouse_wait(1);
    outb(MOUSE_COMMAND_PORT, MOUSE_CMD_ENABLE_AUX);

    // Enable IRQ12
    mouse_wait(1);
    outb(MOUSE_COMMAND_PORT, MOUSE_CMD_READ_CTR);
    u8int status = mouse_read();
    status |= 0x02;
    mouse_wait(1);
    outb(MOUSE_COMMAND_PORT, MOUSE_CMD_WRITE_CTR);
    mouse_wait(1);
    outb(MOUSE_DATA_PORT, status);

    // Set defaults and enable
    mouse_write(MOUSE_DEV_SET_DEFAULTS);
    mouse_read();
    mouse_write(MOUSE_DEV_ENABLE);
    mouse_read();

    register_irq_handler(12, mouse_handler);
}
