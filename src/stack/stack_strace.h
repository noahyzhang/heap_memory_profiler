/**
 * @file stack_strace.h
 * @author noahyzhang
 * @brief 
 * @version 0.1
 * @date 2023-05-04
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef SRC_STACK_STACK_STRACE_H_
#define SRC_STACK_STACK_STRACE_H_

namespace heap_memory_profiler {

extern int get_stack_frames(void** result, int* sizes, int max_depth, int skip_count);

extern int get_stack_frames_with_context(void** result, int* size, int max_depth, int skip_count, const void* uc);

extern int get_stack_strace(void** result, int max_depth, int skip_count);

extern int get_stack_trace_with_context(void** result, int max_depth, int skip_count, const void* uc);

}  // namespace heap_memory_profiler

#endif  // SRC_STACK_STACK_STRACE_H_
