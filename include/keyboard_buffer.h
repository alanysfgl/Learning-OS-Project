#ifndef KEYBOARD_BUFFER_H
#define KEYBOARD_BUFFER_H

void kbd_buffer_init();
void kbd_buffer_push(char c);
int kbd_buffer_pop();

#endif
