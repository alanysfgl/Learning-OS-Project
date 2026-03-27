// ipc.c
#include "ipc.h"

#define IPC_QUEUE_SIZE 32

static u32int queue[IPC_QUEUE_SIZE];
static int head = 0;
static int tail = 0;

void ipc_init(void) {
    head = 0;
    tail = 0;
}

static int ipc_full(void) {
    return ((head + 1) % IPC_QUEUE_SIZE) == tail;
}

static int ipc_empty(void) {
    return head == tail;
}

int ipc_send(u32int msg) {
    if (ipc_full()) return -1;
    queue[head] = msg;
    head = (head + 1) % IPC_QUEUE_SIZE;
    return 0;
}

int ipc_recv(u32int *msg) {
    if (!msg || ipc_empty()) return -1;
    *msg = queue[tail];
    tail = (tail + 1) % IPC_QUEUE_SIZE;
    return 0;
}
