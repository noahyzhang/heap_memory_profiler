/**
 * @file private_memory_manager.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-05-05
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef SRC_COMMON_PRIVATE_MEMORY_MANAGER_H_
#define SRC_COMMON_PRIVATE_MEMORY_MANAGER_H_

#include <string.h>
#include "collect/heap_profiler.h"

namespace heap_memory_profiler {

class PrivateMemoryManager {
private:
    PrivateMemoryManager() = default;
public:
    ~PrivateMemoryManager() = default;
    PrivateMemoryManager(const PrivateMemoryManager&) = delete;
    PrivateMemoryManager& operator=(const PrivateMemoryManager&) = delete;
    PrivateMemoryManager(PrivateMemoryManager&&) = delete;
    PrivateMemoryManager& operator=(PrivateMemoryManager&&) = delete;

    static PrivateMemoryManager& get_instance() {
        static PrivateMemoryManager instance;
        return instance;
    }

public:
    void* alloc(size_t size) {
        if (real_malloc == nullptr) {
            return nullptr;
        }
        return real_malloc(size);
    }

    void dealloc(void* ptr) {
        if (real_free == nullptr) {
            return;
        }
        return real_free(ptr);
    }
};

}  // namespace heap_memory_profiler

#endif  // SRC_COMMON_PRIVATE_MEMORY_MANAGER_H_
