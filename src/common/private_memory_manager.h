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

namespace heap_memory_profiler {

// 内部私有内存字节数: 2M
#define MAX_PRIVATE_MEMORY_BYTES (2 * 1024 * 1024)

class PrivateMemoryManager {
private:
    PrivateMemoryManager() {
        memset(private_memory, 0, MAX_PRIVATE_MEMORY_BYTES);
        cur_memory_pos = private_memory;
    }
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
        // 这 4 字节存储用户申请内存大小
        int wrap_size = size + 4;
        if (size == 0 || wrap_size > cur_memory_bytes) {
            return nullptr;
        }
        (*(uint32_t*)cur_memory_pos) = size;
        char* res = cur_memory_pos + 4;
        cur_memory_pos += wrap_size;
    }

private:
    char private_memory[MAX_PRIVATE_MEMORY_BYTES];
    int cur_memory_bytes{MAX_PRIVATE_MEMORY_BYTES};
    char* cur_memory_pos{nullptr};
};

}  // namespace heap_memory_profiler

#endif  // SRC_COMMON_PRIVATE_MEMORY_MANAGER_H_
