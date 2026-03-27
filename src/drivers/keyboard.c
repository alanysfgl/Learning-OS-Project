#include "keyboard.h"
#include "keyboard_buffer.h"
#include "input.h"
#include "irq.h"
#include "common.h"
#include "monitor.h"
#include "serial.h"

// Klavye tuş haritası (US Layout)
static unsigned char kbdus[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

static unsigned char kbdus_shift[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|',
    'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' '
};

static int shift_pressed = 0;
static int caps_lock = 0;

static void keyboard_handler(struct registers* regs) {
    (void)regs;
    unsigned char scancode = inb(0x60);
    if (scancode & 0x80) {
        // Key release
        unsigned char released = scancode & 0x7F;
        if (released == 0x2A || released == 0x36) {
            shift_pressed = 0;
        }
        return;
    }

    // Key press
    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = 1;
        return;
    }
    if (scancode == 0x3A) {
        caps_lock = !caps_lock;
        return;
    }

    if (scancode < 128) {
        char c = shift_pressed ? kbdus_shift[scancode] : kbdus[scancode];
        if (c >= 'a' && c <= 'z') {
            if (caps_lock ^ shift_pressed) {
                c = (char)(c - 'a' + 'A');
            }
        } else if (c >= 'A' && c <= 'Z') {
            if (!(caps_lock ^ shift_pressed)) {
                c = (char)(c - 'A' + 'a');
            }
        }
        if (c) {
            kbd_buffer_push(c);
            input_push_key(c);
        }
    }
}

void keyboard_init() {
    kbd_buffer_init();
    register_irq_handler(1, keyboard_handler);
}

int keyboard_getchar() {
    int c = kbd_buffer_pop();
    if (c >= 0) return c;
    c = serial_read_char();
    return c;
}

int keyboard_readline(char *out, int max) {
    int idx = 0;
    if (max <= 0) return 0;

    while (idx < max - 1) {
        int c;
        while ((c = keyboard_getchar()) < 0) {
            asm volatile("sti; hlt");
        }

        if (c == '\r' || c == '\n') {
            monitor_put('\n');
            break;
        }
        if (c == '\b' || c == 0x7F) {
            if (idx > 0) {
                idx--;
                monitor_write("\b \b");
            }
            continue;
        }
        monitor_put((char)c);
        out[idx++] = (char)c;
    }

    out[idx] = '\0';
    return idx;
}
