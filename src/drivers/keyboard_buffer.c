#include "keyboard_buffer.h"

#define KBD_BUF_SIZE 128

static char buffer[KBD_BUF_SIZE];
static int head = 0;
static int tail = 0;

void kbd_buffer_init() {
    head = 0;
    tail = 0;
}

void kbd_buffer_push(char c) {
    int next = (head + 1) % KBD_BUF_SIZE;
    if (next == tail) {
        return; // buffer full, drop
    }
    buffer[head] = c;
    head = next;
}

int kbd_buffer_pop() {
    if (tail == head) {
        return -1;
    }
    char c = buffer[tail];
    tail = (tail + 1) % KBD_BUF_SIZE;
    return (int)c;
}
