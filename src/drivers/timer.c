#include "timer.h"
#include "pit.h"
#include "irq.h"
#include "common.h"
#include "monitor.h"
#include "scheduler.h"

static volatile u32int timer_ticks = 0;

static void timer_handler(struct registers* regs) {
    (void)regs;
    timer_ticks++;
    scheduler_tick();
}

void timer_init(u32int frequency) {
    pit_init(frequency);
    register_irq_handler(0, timer_handler);
}
