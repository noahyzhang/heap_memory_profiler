/**
 * @file sample_stats.h
 * @author noahyzhang
 * @brief 
 * @version 0.1
 * @date 2023-05-02
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include <stdint.h>

namespace heap_memory_profiler {

struct SampleStats {
    int32_t call_alloc_num;
    int32_t call_dealloc_num;
    int64_t alloc_size;
    int64_t dealloc_size;

    bool is_equivalent(const SampleStats& other) {
        return ((call_alloc_num - call_dealloc_num == other.call_alloc_num - other.call_dealloc_num)
            && (alloc_size - dealloc_size == other.alloc_size - other.dealloc_size));
    }
};

struct SampleBucket : public SampleStats {
    static const int max_stack_depth = 32;
    uintptr_t hash;
    int depth;
    const void** stack;
    SampleBucket* next;
};

}  // namespace heap_memory_profiler
