/**
 * @file concurrent_list.cpp
 * @auther noahyzhang
 * @brief
 * @version 0.1
 * @date 2023/5/3
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "basic_types.h"
#include "spinlock.h"
#include "concurrent_list.h"

namespace heap_memory_profiler {

template <typename T>
bool ConcurrentList<T>::add(T value) {
    uintptr_t val = bit_cast<uintptr_t>(value);
    if (val == 0) {
        return false;
    }
    LockGuard<SpinLock> guard(&concurrent_list_spinlock);
    int index = 0;
    while ((index < concurrentListMaxValue)
        && (priv_data[index].load() != 0)) {
        index++;
    }
    if (index == concurrentListMaxValue) {
        return false;
    }
    auto prev_num_hooks = priv_end.load();
    priv_data[index].store(val);
    if (prev_num_hooks <= index) {
        priv_end.store(index+1);
    }
    return true;
}

template <typename T>
bool ConcurrentList<T>::remove(T value) {
    if (value == 0) {
        return false;
    }
    LockGuard<SpinLock> guard(&concurrent_list_spinlock);
    auto hooks_end = priv_end.load();
    int index = 0;
    while ((index < hooks_end)
        && (value != bit_cast<T>(priv_data[index].load()))) {
        index++;
    }
    if (index == hooks_end) {
        return false;
    }
    priv_data[index].store(0);
    fixup_priv_end_locked();
    return true;
}

template <typename T>
bool ConcurrentList<T>::empty() const {
    return priv_end.load() == 0;
}

template <typename T>
int ConcurrentList<T>::traverse(T* output_array, int n) const {
    auto hooks_end = priv_end.load();
    int actual_hooks_end = 0;
    for (int i = 0; i < hooks_end && n > 0; ++i) {
        auto data = priv_data[i].load();
        if (data != 0) {
            *output_array++ = bit_cast<T>(data);
            ++actual_hooks_end;
            --n;
        }
    }
    return actual_hooks_end;
}

template <typename T>
void ConcurrentList<T>::fixup_priv_end_locked() {
    auto hooks_end = priv_end.load();
    while ((hooks_end > 0)
        && priv_data[hooks_end-1].load() == 0) {
        --hooks_end;
    }
    priv_end.store(hooks_end);
}

}  // namespace heap_memory_profiler
