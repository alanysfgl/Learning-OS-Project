#ifndef IRQ_H
#define IRQ_H

#include "idt.h"

void register_irq_handler(u8int irq, irq_handler_t handler);
void irq_handler(struct registers regs);

#endif
