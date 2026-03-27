// assert.h
#ifndef ASSERT_H
#define ASSERT_H

#include "panic.h"

#define assert(cond) do { \
    if (!(cond)) { \
        panic("assertion failed: " #cond, __FILE__, __LINE__); \
    } \
} while (0)

#endif
