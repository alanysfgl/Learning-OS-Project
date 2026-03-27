// input.h
#ifndef INPUT_H
#define INPUT_H

#include "common.h"

enum input_type {
    INPUT_KEY = 1,
    INPUT_MOUSE = 2
};

struct input_event {
    u8int type;
    u8int flags;
    s16int a;
    s16int b;
};

void input_init(void);
void input_push_key(char c);
void input_push_mouse(s8int dx, s8int dy, u8int buttons);
int input_pop(struct input_event *out);

#endif
