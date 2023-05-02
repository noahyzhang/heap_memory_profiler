/**
 * @file record_manager.h
 * @author noahyzhang
 * @brief 
 * @version 0.1
 * @date 2023-05-02
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include <stddef.h>
#include <string>
#include <unordered_map>
#include "sample_stats.h"

namespace heap_memory_profiler {

class AllocValue {
public:
    void set_bucket(SampleBucket* bucket) {
        bucket_rep_ = reinterpret_cast<uintptr_t>(bucket);
    }

    SampleBucket* get_bucket() const {
        return reinterpret_cast<SampleBucket*>(bucket_rep_ & ~uintptr_t(mask_));
    }

    void set_alloc_bytes(uint64_t bytes) {
        bytes_ = bytes;
    }

    uint64_t get_alloc_bytes() const {
        return bytes_;
    }

private:
    uintptr_t bucket_rep_{0};
    uint64_t bytes_{0};

    static const int live_ = 1;
    static const int ignore_ = 2;
    static const int mask_ = live_ | ignore_;
};

class RecordManager {
public:

public:
    void record_alloc(const void* ptr, size_t bytes, int stack_depth, const void* const call_stack[]);
    void record_dealloc(const void* ptr);

    const SampleBucket& get_total_mem_info() const {
        return total_mem_info_;
    }

    int fill_ordered_profile(std::ostream& fstream);

public:
    static int get_caller_stack_trace(int skip_count, void** stack);

private:
    SampleBucket* get_bucket(int depth, const void* const key[]);
    std::string parse_bucket(const SampleBucket& bucket, const char* extra);
    SampleBucket** make_sorted_bucket_list() const;

private:
    SampleBucket** bucket_table_{nullptr};
    uint64_t bucket_num_{0};

    SampleBucket total_mem_info_;

    bool is_profile_mmap_;

    std::unordered_map<const void*, AllocValue> address_mp_;

    Allocator alloc_;
    Deallocator dealloc_;
};

}  // namespace heap_memory_profiler
