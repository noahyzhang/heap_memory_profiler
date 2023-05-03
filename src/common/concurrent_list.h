/**
 * @file concurrent_list.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-05-03
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef SRC_COMMON_CONCURRENT_LIST_H_
#define SRC_COMMON_CONCURRENT_LIST_H_

#include <atomic>

namespace heap_memory_profiler {

static const int concurrentListCapacity = 8;
static const int concurrentListMaxValue = 7;

template <typename T>
class ConcurrentList {
public:
    bool add(T value);
    bool remove(T value);
    bool empty() const;
    int traverse(T* output_array, int n) const;

private:
    void fixup_priv_end_locked();

private:
    std::atomic<uintptr_t> priv_end;
    std::atomic<uintptr_t> priv_data[concurrentListCapacity];
};

}  // namespace heap_memory_profiler

#endif  // SRC_COMMON_CONCURRENT_LIST_H_
