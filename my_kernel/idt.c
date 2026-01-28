#include "idt.h"

// common.h içinde outb/inb tanımlı değilse diye buraya da ekliyoruz
static inline void outb(unsigned short port, unsigned char value) {
    asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}

static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    asm volatile ("inb %1, %0" : "=a" (ret) : "dN" (port));
    return ret;
}

// Kendi memory_set fonksiyonumuz (string.h hatasını önlemek için)
void* memory_set(void *dest, int val, unsigned int len) {
    unsigned char *temp = (unsigned char *)dest;
    for ( ; len != 0; len--) *temp++ = val;
    return dest;
}

// Assembly'den gelecek fonksiyonlar
extern void idt_flush(unsigned int);
extern void isr0();  // Sıfıra bölme
extern void irq1();  // Klavye
extern void monitor_put(char c); // monitor.c'den al

struct idt_entry_struct idt_entries[256];
struct idt_ptr_struct   idt_ptr;

// Klavye tuş haritası (US Layout)
unsigned char kbdus[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

// Kayıt ekleme fonksiyonu
void idt_set_gate(unsigned char num, unsigned int base, unsigned short sel, unsigned char flags) {
    idt_entries[num].base_low = base & 0xFFFF;
    idt_entries[num].base_high = (base >> 16) & 0xFFFF;
    idt_entries[num].sel     = sel;
    idt_entries[num].always0 = 0;
    idt_entries[num].flags   = flags;
}

// PIC Yeniden Haritalandırma (IRQ'ları 32'den başlatır)
void init_pic() {
    outb(0x20, 0x11); outb(0xA0, 0x11);
    outb(0x21, 0x20); outb(0xA1, 0x28);
    outb(0x21, 0x04); outb(0xA1, 0x02);
    outb(0x21, 0x01); outb(0xA1, 0x01);
    
    // KRİTİK: Sadece klavyeye (IRQ 1) izin ver, diğerlerini maskele
    // 0xFD -> 11111101 (Sadece 1. bit yani IRQ 1 açık)
    outb(0x21, 0xFD); 
    outb(0xA1, 0xFF); // Slave PIC tamamen kapalı
}
// Tüm kesmelerin toplandığı ana yakalayıcı
struct registers {
   unsigned int ds;
   unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
   unsigned int int_no, err_code;
   unsigned int eip, cs, eflags, useresp, ss;
};

void irq_handler(struct registers regs) {
    // Klavye Kesmesi (IRQ 1 -> IDT 33)
    if (regs.int_no == 33) {
        unsigned char scancode = inb(0x60);
        if (scancode < 128) {
            char c = kbdus[scancode];
            monitor_put(c);
        }
    }
    // Sıfıra Bölme Hatası (ISR 0)
    else if (regs.int_no == 0) {
        char* video_memory = (char*) 0xB8000;
        const char* msg = "HATA: SIFIRA BOLME!";
        for(int i=0; msg[i]; i++) {
            video_memory[i*2] = msg[i];
            video_memory[i*2+1] = 0x4F; // Kırmızı arka plan
        }
    }

    // PIC'e "İşlem Tamam" sinyali gönder (EOI)
    if (regs.int_no >= 32) {
        if (regs.int_no >= 40) outb(0xA0, 0x20);
        outb(0x20, 0x20);
    }
}

// Ana Başlatıcı
void init_idt() {
    idt_ptr.limit = sizeof(struct idt_entry_struct) * 256 - 1;
    idt_ptr.base  = (unsigned int)&idt_entries;

    memory_set(&idt_entries, 0, sizeof(struct idt_entry_struct) * 256);

    init_pic();

    // Kesmeleri tabloya ekle
    idt_set_gate(0, (unsigned int)isr0, 0x08, 0x8E);  // Hata yakalayıcı
    idt_set_gate(33, (unsigned int)irq1, 0x08, 0x8E); // Klavye yakalayıcı

    idt_flush((unsigned int)&idt_ptr);
    
    // Kesmeleri dünya genelinde aç
    asm volatile("sti");
}