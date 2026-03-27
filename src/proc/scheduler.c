// scheduler.c
#include "scheduler.h"
#include "task.h"
#include "heap.h"
#include "logger.h"

#define MAX_TASKS 4
#define STACK_SIZE 4096

enum task_state {
    TASK_UNUSED = 0,
    TASK_RUNNABLE,
    TASK_ZOMBIE
};

struct task {
    struct task_context ctx;
    u32int stack_base;
    int pid;
    int exit_code;
    enum task_state state;
};

static struct task tasks[MAX_TASKS];
static int task_count = 0;
static int current_task = -1; // -1 means kernel context
static int active_tasks = 0;
static int scheduling_enabled = 0;
static volatile int need_resched = 0;
static struct task_context kernel_ctx;
static int next_pid = 1;

void scheduler_init(void) {
    task_count = 0;
    current_task = -1;
    active_tasks = 0;
    scheduling_enabled = 0;
    need_resched = 0;
    next_pid = 1;
    for (int i = 0; i < MAX_TASKS; i++) {
        tasks[i].state = TASK_UNUSED;
        tasks[i].stack_base = 0;
        tasks[i].pid = 0;
        tasks[i].exit_code = 0;
        tasks[i].ctx.irq_esp = 0;
    }
}

int scheduler_add(void (*entry)(void)) {
    if (!entry) {
        return -1;
    }

    int slot = -1;
    for (int i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].state == TASK_UNUSED) {
            slot = i;
            break;
        }
    }
    if (slot < 0) {
        return -1;
    }

    u32int stack_base = (u32int)kmalloc(STACK_SIZE);
    if (!stack_base) {
        log_err("Task stack alloc failed.\n");
        return -1;
    }

    u32int stack_top = stack_base + STACK_SIZE;
    task_context_init(&tasks[slot].ctx, (u32int)entry, stack_top);
    tasks[slot].stack_base = stack_base;
    tasks[slot].pid = next_pid++;
    tasks[slot].exit_code = 0;
    tasks[slot].state = TASK_RUNNABLE;

    active_tasks++;
    if (slot >= task_count) {
        task_count = slot + 1;
    }
    return tasks[slot].pid;
}

void scheduler_start(void) {
    if (active_tasks == 0) {
        log_warn("Scheduler start: no tasks.\n");
        return;
    }
    scheduling_enabled = 1;
    current_task = -1;
    need_resched = 1;
    asm volatile("int $0x20");
}

void scheduler_tick(void) {
    if (!scheduling_enabled || task_count < 2) return;
    static u32int ticks = 0;
    ticks++;
    if ((ticks % 10) == 0) {
        need_resched = 1;
    }
}

void task_yield(void) {
    if (!scheduling_enabled || task_count < 2) return;
    need_resched = 1;
    asm volatile("int $0x20");
}

void scheduler_exit(int code) {
    (void)code;
    if (!scheduling_enabled || task_count == 0) return;

    int prev = current_task;
    tasks[prev].state = TASK_ZOMBIE;
    tasks[prev].exit_code = code;
    if (active_tasks > 0) {
        active_tasks--;
    }

    if (active_tasks == 0) {
        need_resched = 1;
        asm volatile("int $0x20");
        return;
    }
    need_resched = 1;
    asm volatile("int $0x20");
}

u32int scheduler_switch_from_isr(u32int current_esp) {
    if (!scheduling_enabled || task_count == 0) {
        return current_esp;
    }

    if (current_task >= 0) {
        tasks[current_task].ctx.irq_esp = current_esp;
    } else {
        kernel_ctx.irq_esp = current_esp;
    }

    if (!need_resched) {
        return current_esp;
    }
    need_resched = 0;

    if (active_tasks == 0) {
        current_task = -1;
        scheduling_enabled = 0;
        return kernel_ctx.irq_esp;
    }

    int next = current_task;
    for (int i = 0; i < task_count; i++) {
        next = (next + 1) % task_count;
        if (tasks[next].state == TASK_RUNNABLE) {
            current_task = next;
            return tasks[next].ctx.irq_esp;
        }
    }

    current_task = -1;
    return kernel_ctx.irq_esp;
}

int scheduler_wait(int pid, int *exit_code) {
    for (int i = 0; i < task_count; i++) {
        if (tasks[i].pid == pid && tasks[i].state == TASK_ZOMBIE) {
            if (exit_code) {
                *exit_code = tasks[i].exit_code;
            }
            if (tasks[i].stack_base) {
                kfree((void *)tasks[i].stack_base);
                tasks[i].stack_base = 0;
            }
            tasks[i].state = TASK_UNUSED;
            return 0;
        }
    }
    return -1;
}
