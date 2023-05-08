/**
 * @file heap_profiler.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-05-08
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef SRC_COLLECT_HEAP_PROFILER_H_
#define SRC_COLLECT_HEAP_PROFILER_H_

#include <stddef.h>

namespace heap_memory_profiler {

using malloc_type = void*(*)(size_t);
using free_type = void(*)(void*);
using cfree_type = void(*)(void*);
using calloc_type = void*(*)(size_t, size_t);
using realloc_type = void*(*)(void*, size_t);
using memalign_type = void*(*)(size_t, size_t);
using posix_memalign_type = int(*)(void**, size_t, size_t);
using aligned_alloc_type = void*(*)(size_t, size_t);
using valloc_type = void*(*)(size_t);
using pvalloc_type = void*(*)(size_t);

malloc_type real_malloc;
free_type real_free;
cfree_type real_cfree;
calloc_type real_calloc;
realloc_type real_realloc;
memalign_type real_memalign;
posix_memalign_type real_posix_memalign;
aligned_alloc_type real_aligned_alloc;
valloc_type real_valloc;
pvalloc_type real_pvalloc;


}  // namespace heap_memory_profiler

#endif  // SRC_COLLECT_HEAP_PROFILER_H_
