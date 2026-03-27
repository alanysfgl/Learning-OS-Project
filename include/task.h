// task.h
#ifndef TASK_H
#define TASK_H

#include "common.h"

struct task_context {
    u32int irq_esp;
};

void task_context_init(struct task_context *ctx, u32int entry, u32int stack_top);
void task_start(u32int new_esp);

#endif
