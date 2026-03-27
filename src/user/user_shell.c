// user_shell.c
#include "userlib.h"
#include "libc.h"

void user_shell_entry(void) {
    char line[64];
    for (;;) {
        const char *prompt = "user> ";
        sys_write(1, prompt, 6);
        int n = 0;
        while (n < (int)sizeof(line) - 1) {
            char ch = 0;
            int r = sys_read(0, (u8int *)&ch, 1);
            if (r <= 0) continue;

            if (ch == '\r') ch = '\n';
            if (ch == '\n') {
                sys_write(1, (const u8int *)"\n", 1);
                break;
            }
            if (ch == '\b' || ch == 0x7F) {
                if (n > 0) {
                    n--;
                    sys_write(1, (const u8int *)"\b \b", 3);
                }
                continue;
            }

            line[n++] = ch;
            sys_write(1, (const u8int *)&ch, 1);
        }
        line[n] = 0;
        if (n == 0) continue;
        if (strcmp(line, "exit\n") == 0 || strcmp(line, "exit") == 0) {
            sys_exit(0);
        }
        sys_write(1, "echo: ", 6);
        sys_write(1, line, (u32int)n);
    }
}
