// ipc.h
#ifndef IPC_H
#define IPC_H

#include "common.h"

void ipc_init(void);
int ipc_send(u32int msg);
int ipc_recv(u32int *msg);

#endif
