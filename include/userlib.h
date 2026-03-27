// userlib.h
#ifndef USERLIB_H
#define USERLIB_H

#include "common.h"

int sys_read(int fd, void *buf, u32int size);
int sys_write(int fd, const void *buf, u32int size);
int sys_open(const char *path);
void sys_exit(int code);
int sys_ipc_send(u32int msg);
int sys_ipc_recv(u32int *msg);

#endif
