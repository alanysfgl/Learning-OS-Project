// init.c
#include "init.h"
#include "userlib.h"
#include "shell.h"
#include "usermode.h"
#include "user_shell.h"
#include "memory.h"

void init_task(void) {
    const char *banner = "init: starting shell\n";
    sys_write(1, banner, 22);
    u32int stack = frame_alloc_contiguous(1);
    if (!stack) {
        sys_write(1, "init: no stack\n", 15);
        shell_run();
        sys_exit(1);
    }
    enter_user_mode((u32int)user_shell_entry, stack + 4096);
    shell_run();
    sys_exit(0);
}
