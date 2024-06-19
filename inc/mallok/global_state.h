#pragma once

#include <stdbool.h>
#include <mallok/log.h>
#include <mallok/allocator.h>

typedef struct {
    bool initialized;
    size_t page_size;
    log_level logging_level;
    allocator alloc;
} global_state;

extern global_state state;

void __attribute__((constructor(101))) global_state_assert_init(void);
void __attribute__((destructor(101))) global_state_assert_deinit(void);
