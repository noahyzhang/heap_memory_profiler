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

using MallocHook_NewHook = void(*)(const void* ptr, size_t size);

#ifdef __cpluscplus
}
#endif

#endif  // SRC_COLLECT_MALLOC_HOOK_C_H_


