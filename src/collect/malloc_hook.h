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

#include "malloc_hook_c.h"

namespace heap_memory_profiler {

class MallocHook {
public:
    inline static bool add_new_hook();

};

}  // namespace heap_memory_profiler

#endif  // SRC_COLLECT_MALLOC_HOOK_H_
