#include "common/concurrent_list.h"
#include "malloc_hook.h"

namespace heap_memory_profiler {

// TODO
ConcurrentList<MallocHook_AllocHook> alloc_hooks_;
ConcurrentList<MallocHook_DeallocHook> dealloc_hooks_;

inline bool MallocHook::add_alloc_hook(MallocHook_AllocHook hook) {
    return alloc_hooks_.add(hook);
}

inline bool MallocHook::add_dealloc_hook(MallocHook_DeallocHook hook) {
    return dealloc_hooks_.add(hook);
}

#define INVOKE_HOOKS(HookType, hook_list, args) \
    do { \
        HookType hooks[concurrentListMaxValue];  \
        int num_hooks = hook_list.traverse(hooks, concurrentListMaxValue); \
        for (int i = 0; i < num_hooks; ++i) {            \
            (*hooks[i])args;                                                 \
        }  \
    } while (0)


void MallocHook::invoke_alloc_hook_slow(const void* ptr, size_t size) {
    INVOKE_HOOKS(MallocHook_AllocHook, alloc_hooks_, (ptr, size));
}

void MallocHook::invoke_dealloc_hook_slow(const void* ptr) {
    INVOKE_HOOKS(MallocHook_DeallocHook, dealloc_hooks_, (ptr));
}

}  // namespace heap_memory_profiler
