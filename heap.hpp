#pragma once

#include <stddef.h>
#include <stdint.h>

struct HeapItem {
    uint64_t val = 0;
    size_t* ref = nullptr;
};

void heap_update(HeapItem* item, size_t pos, size_t len);