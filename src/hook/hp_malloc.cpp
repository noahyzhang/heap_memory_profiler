#include <stddef.h>
#include <errno.h>
#include "collect/heap_profiler.h"
#include "collect/malloc_hook.h"

extern "C"
void* hp_malloc(size_t size) {
    if (heap_memory_profiler::real_malloc == nullptr) {
        return nullptr;
    }
    void* ptr = heap_memory_profiler::real_malloc(size);
    if (ptr != nullptr) {
        heap_memory_profiler::MallocHook::invoke_alloc_hook_slow(ptr, size);
    }
    return ptr;
}

extern "C"
void hp_free(void* ptr) {
    if (heap_memory_profiler::real_free == nullptr || ptr == nullptr) {
        return;
    }
    heap_memory_profiler::MallocHook::invoke_dealloc_hook_slow(ptr);
    heap_memory_profiler::real_free(ptr);
}

extern "C"
void hp_cfree(void* ptr) {
    if (heap_memory_profiler::real_cfree == nullptr || ptr == nullptr) {
        return;
    }
    heap_memory_profiler::MallocHook::invoke_dealloc_hook_slow(ptr);
    heap_memory_profiler::real_cfree(ptr);
}

extern "C"
void* hp_calloc(size_t nmemb, size_t size) {
    if (heap_memory_profiler::real_calloc == nullptr) {
        return nullptr;
    }
    void* ptr = heap_memory_profiler::real_calloc(nmemb, size);
    if (ptr != nullptr) {
        heap_memory_profiler::MallocHook::invoke_alloc_hook_slow(ptr, nmemb * size);
    }
    return ptr;
}

extern "C"
void* hp_realloc(void* old_ptr, size_t new_size) {
    if (heap_memory_profiler::real_realloc == nullptr
        || heap_memory_profiler::real_malloc == nullptr
        || heap_memory_profiler::real_free == nullptr) {
        return nullptr;
    }
    if (old_ptr == nullptr) {
        void* result = heap_memory_profiler::real_malloc(new_size);
        if (result != nullptr) {
            heap_memory_profiler::MallocHook::invoke_alloc_hook_slow(result, new_size);
        }
        return result;
    }
    if (new_size == 0) {
        heap_memory_profiler::MallocHook::invoke_dealloc_hook_slow(old_ptr);
        heap_memory_profiler::real_free(old_ptr);
        return nullptr;
    }
    return heap_memory_profiler::real_realloc(old_ptr, new_size);
}

extern "C"
void* hp_memalign(size_t alignment, size_t size) {
    if (heap_memory_profiler::real_memalign == nullptr) {
        return nullptr;
    }
    void* ptr = heap_memory_profiler::real_memalign(alignment, size);
    if (ptr != nullptr) {
        heap_memory_profiler::MallocHook::invoke_alloc_hook_slow(ptr, size);
    }
    return ptr;
}

extern "C"
void* hp_aligned_alloc(size_t alignment, size_t size) {
    if (heap_memory_profiler::real_aligned_alloc == nullptr) {
        return nullptr;
    }
    void* ptr = heap_memory_profiler::real_aligned_alloc(alignment, size);
    if (ptr != nullptr) {
        heap_memory_profiler::MallocHook::invoke_alloc_hook_slow(ptr, size);
    }
    return ptr;
}

extern "C"
int hp_posix_memalign(void** memptr, size_t alignment, size_t size) {
    if (heap_memory_profiler::real_posix_memalign == nullptr) {
        return EINVAL;
    }
    if (memptr == nullptr) {
        return EINVAL;
    }
    int res = heap_memory_profiler::real_posix_memalign(memptr, alignment, size);
    if (res == 0) {
        heap_memory_profiler::MallocHook::invoke_alloc_hook_slow(*memptr, size);
    }
    return res;
}

extern "C"
void* hp_valloc(size_t size) {
    if (heap_memory_profiler::real_valloc == nullptr) {
        return nullptr;
    }
    void* result = heap_memory_profiler::real_valloc(size);
    if (result != nullptr) {
        heap_memory_profiler::MallocHook::invoke_alloc_hook_slow(result, size);
    }
    return result;
}

extern "C"
void* hp_pvalloc(size_t size) {
    if (heap_memory_profiler::real_pvalloc == nullptr) {
        return nullptr;
    }
    void* result = heap_memory_profiler::real_pvalloc(size);
    if (result != nullptr) {
        heap_memory_profiler::MallocHook::invoke_alloc_hook_slow(result, size);
    }
    return result;
}

