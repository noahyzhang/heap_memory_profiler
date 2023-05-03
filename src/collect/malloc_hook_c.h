/**
 * @file malloc_hook_c.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-05-02
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef SRC_COLLECT_MALLOC_HOOK_C_H_
#define SRC_COLLECT_MALLOC_HOOK_C_H_

#include <stddef.h>

#ifdef __cpluscplus
extern "C" {
#endif

using MallocHook_AllocHook = void(*)(const void* ptr, size_t size);
using MallocHook_DeallocHook = void(*)(const void* ptr);

int MallocHook_add_alloc_hook(MallocHook_AllocHook hook);
int MallocHook_add_dealloc_hook(MallocHook_DeallocHook hook);

#ifdef __cpluscplus
}
#endif

#endif  // SRC_COLLECT_MALLOC_HOOK_C_H_


