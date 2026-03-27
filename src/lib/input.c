// input.c
#include "input.h"

#define INPUT_BUF_SIZE 64

static struct input_event buffer[INPUT_BUF_SIZE];
static int head = 0;
static int tail = 0;

void input_init(void) {
    head = 0;
    tail = 0;
}

static int input_full(void) {
    return ((head + 1) % INPUT_BUF_SIZE) == tail;
}

static int input_empty(void) {
    return head == tail;
}

static void input_push(struct input_event *ev) {
    if (input_full()) {
        return;
    }
    buffer[head] = *ev;
    head = (head + 1) % INPUT_BUF_SIZE;
}

void input_push_key(char c) {
    struct input_event ev;
    ev.type = INPUT_KEY;
    ev.flags = 0;
    ev.a = (s16int)c;
    ev.b = 0;
    input_push(&ev);
}

void input_push_mouse(s8int dx, s8int dy, u8int buttons) {
    struct input_event ev;
    ev.type = INPUT_MOUSE;
    ev.flags = buttons;
    ev.a = (s16int)dx;
    ev.b = (s16int)dy;
    input_push(&ev);
}

int input_pop(struct input_event *out) {
    if (!out || input_empty()) return -1;
    *out = buffer[tail];
    tail = (tail + 1) % INPUT_BUF_SIZE;
    return 0;
}
