/**
 * @file malloc_hook.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-05-02
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef SRC_COLLECT_MALLOC_HOOK_H_
#define SRC_COLLECT_MALLOC_HOOK_H_

namespace heap_memory_profiler {

using MallocHook_AllocHook = void(*)(const void* ptr, size_t size);
using MallocHook_DeallocHook = void(*)(const void* ptr);

class MallocHook {
public:
    inline static bool add_alloc_hook(MallocHook_AllocHook hook);
    inline static bool add_dealloc_hook(MallocHook_DeallocHook hook);

    inline static bool remove_alloc_hook(MallocHook_AllocHook hook);
    inline static bool remove_dealloc_hook(MallocHook_DeallocHook hook);

    static void invoke_alloc_hook_slow(const void* ptr, size_t size);
    static void invoke_dealloc_hook_slow(const void* ptr);

};

}  // namespace heap_memory_profiler

#endif  // SRC_COLLECT_MALLOC_HOOK_H_
