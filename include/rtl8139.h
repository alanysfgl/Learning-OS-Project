// rtl8139.h
#ifndef RTL8139_H
#define RTL8139_H

#include "common.h"

int rtl8139_init(void);
int rtl8139_send(const void *data, u32int len);

#endif
