// scheduler.h
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "common.h"

void scheduler_init(void);
int scheduler_add(void (*entry)(void));
void scheduler_start(void);
void scheduler_tick(void);
void task_yield(void);
void scheduler_exit(int code);
u32int scheduler_switch_from_isr(u32int current_esp);
int scheduler_wait(int pid, int *exit_code);

#endif
