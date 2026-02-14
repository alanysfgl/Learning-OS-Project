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
extern void irq0();  // PIT timer
extern void irq1();  // Klavye
extern void monitor_put(char c); // monitor.c'den al
extern void monitor_write(char *text);

// CPU exception ISR fonksiyonları (0-31)
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

struct idt_entry_struct idt_entries[256];
struct idt_ptr_struct   idt_ptr;

volatile u32int timer_ticks = 0;

const char *exception_messages[32] = {
    "Division By Zero", "Debug", "Non Maskable Interrupt", "Breakpoint",
    "Into Detected Overflow", "Out of Bounds", "Invalid Opcode", "No Coprocessor",
    "Double Fault", "Coprocessor Segment Overrun", "Bad TSS", "Segment Not Present",
    "Stack Fault", "General Protection Fault", "Page Fault", "Unknown Interrupt",
    "Coprocessor Fault", "Alignment Check", "Machine Check", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Security Exception", "Reserved"
};


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
    
    // KRİTİK: Timer (IRQ0) ve klavyeye (IRQ1) izin ver, diğerlerini maskele
    // 0xFC -> 11111100 (0. ve 1. bit açık)
    outb(0x21, 0xFC); 
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
    // CPU Exception (ISR 0-31)
    if (regs.int_no < 32) {
        monitor_write("\nEXCEPTION: ");
        monitor_write((char *)exception_messages[regs.int_no]);
        monitor_write("\nSystem halted.\n");

        asm volatile("cli");
        for (;;) {
            asm volatile("hlt");
        }
    }
    // Timer Kesmesi (IRQ 0 -> IDT 32)
    else if (regs.int_no == 32) {
        timer_ticks++;
    }
    // Klavye Kesmesi (IRQ 1 -> IDT 33)
    else if (regs.int_no == 33) {
        unsigned char scancode = inb(0x60);
        if (scancode < 128) {
            char c = kbdus[scancode];
            monitor_put(c);
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

    // CPU exceptions (ISR 0-31)
    idt_set_gate(0,  (unsigned int)isr0,  0x08, 0x8E);
    idt_set_gate(1,  (unsigned int)isr1,  0x08, 0x8E);
    idt_set_gate(2,  (unsigned int)isr2,  0x08, 0x8E);
    idt_set_gate(3,  (unsigned int)isr3,  0x08, 0x8E);
    idt_set_gate(4,  (unsigned int)isr4,  0x08, 0x8E);
    idt_set_gate(5,  (unsigned int)isr5,  0x08, 0x8E);
    idt_set_gate(6,  (unsigned int)isr6,  0x08, 0x8E);
    idt_set_gate(7,  (unsigned int)isr7,  0x08, 0x8E);
    idt_set_gate(8,  (unsigned int)isr8,  0x08, 0x8E);
    idt_set_gate(9,  (unsigned int)isr9,  0x08, 0x8E);
    idt_set_gate(10, (unsigned int)isr10, 0x08, 0x8E);
    idt_set_gate(11, (unsigned int)isr11, 0x08, 0x8E);
    idt_set_gate(12, (unsigned int)isr12, 0x08, 0x8E);
    idt_set_gate(13, (unsigned int)isr13, 0x08, 0x8E);
    idt_set_gate(14, (unsigned int)isr14, 0x08, 0x8E);
    idt_set_gate(15, (unsigned int)isr15, 0x08, 0x8E);
    idt_set_gate(16, (unsigned int)isr16, 0x08, 0x8E);
    idt_set_gate(17, (unsigned int)isr17, 0x08, 0x8E);
    idt_set_gate(18, (unsigned int)isr18, 0x08, 0x8E);
    idt_set_gate(19, (unsigned int)isr19, 0x08, 0x8E);
    idt_set_gate(20, (unsigned int)isr20, 0x08, 0x8E);
    idt_set_gate(21, (unsigned int)isr21, 0x08, 0x8E);
    idt_set_gate(22, (unsigned int)isr22, 0x08, 0x8E);
    idt_set_gate(23, (unsigned int)isr23, 0x08, 0x8E);
    idt_set_gate(24, (unsigned int)isr24, 0x08, 0x8E);
    idt_set_gate(25, (unsigned int)isr25, 0x08, 0x8E);
    idt_set_gate(26, (unsigned int)isr26, 0x08, 0x8E);
    idt_set_gate(27, (unsigned int)isr27, 0x08, 0x8E);
    idt_set_gate(28, (unsigned int)isr28, 0x08, 0x8E);
    idt_set_gate(29, (unsigned int)isr29, 0x08, 0x8E);
    idt_set_gate(30, (unsigned int)isr30, 0x08, 0x8E);
    idt_set_gate(31, (unsigned int)isr31, 0x08, 0x8E);

    // IRQ 0: PIT Timer
    idt_set_gate(32, (unsigned int)irq0, 0x08, 0x8E);
    // IRQ 1: Klavye
    idt_set_gate(33, (unsigned int)irq1, 0x08, 0x8E);

    idt_flush((unsigned int)&idt_ptr);
    
    // Kesmeleri dünya genelinde aç
    asm volatile("sti");
}


u32int get_timer_ticks() {
    return timer_ticks;
}
