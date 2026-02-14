#include "common.h"


// Portlara veri göndermek için gerekli fonksiyon
static inline void outb(unsigned short port, unsigned char value) {
    asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}

// VGA terminal boyutları
#define COLUMNS 80
#define ROWS 25

u16int *video_memory = (u16int *)0xB8000;
u8int cursor_x = 0;
u8int cursor_y = 0;

// İmleci donanımsal olarak hareket ettirir
void update_cursor() {
    u16int cursorLocation = cursor_y * COLUMNS + cursor_x;
    outb(0x3D4, 14);                  // Yüksek byte'ı göndereceğimizi söyle
    outb(0x3D5, cursorLocation >> 8); // Yüksek byte
    outb(0x3D4, 15);                  // Düşük byte'ı göndereceğimizi söyle
    outb(0x3D5, cursorLocation);      // Düşük byte
}

// Ekranı yukarı kaydırır
void scroll() {
    if(cursor_y >= ROWS) {
        u16int blank = 0x20 | (0x0F << 8); // Boşluk karakteri
        for (int i = 0; i < (ROWS - 1) * COLUMNS; i++) {
            video_memory[i] = video_memory[i + COLUMNS];
        }
        for (int i = (ROWS - 1) * COLUMNS; i < ROWS * COLUMNS; i++) {
            video_memory[i] = blank;
        }
        cursor_y = ROWS - 1;
    }
}

// Tek bir karakter yazdırır (Eski kodumuzun gelişmiş hali)
void monitor_put(char c) {
    u8int backColor = 0; // Siyah
    u8int foreColor = 15; // Beyaz
    u16int attribute = (backColor << 12) | (foreColor << 8);

    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
        } else if (cursor_y > 0) {
            cursor_y--;
            cursor_x = COLUMNS - 1;
        }

        u16int *location = video_memory + (cursor_y * COLUMNS + cursor_x);
        *location = ' ' | attribute;
    } else if (c >= ' ') {
        u16int *location = video_memory + (cursor_y * COLUMNS + cursor_x);
        *location = c | attribute;
        cursor_x++;
    }

    if (cursor_x >= COLUMNS) {
        cursor_x = 0;
        cursor_y++;
    }

    scroll();
    update_cursor();
}

// Tüm bir metni yazdırır
void monitor_write(char *text) {
    int i = 0;
    while (text[i]) {
        monitor_put(text[i++]);
    }
}
