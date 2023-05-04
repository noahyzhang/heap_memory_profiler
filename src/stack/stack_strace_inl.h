/**
 * @file stack_strace_inl.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-05-04
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef SRC_COLLECT_STACK_STRACE_INL_H_
#define SRC_COLLECT_STACK_STRACE_INL_H_

namespace heap_memory_profiler {

class CollectStackBase {
public:
    virtual int get_stack_frames(
        void** result, int* size, int max_depth, int skip_count) = 0;

    virtual int get_stack_frames_with_context(
        void** result, int* size, int max_depth, int skip_count, const void* uc) = 0;

    virtual int get_stack_trace(
        void** result, int max_depth, int skip_count) = 0;

    virtual int get_stack_trace_with_context(
        void** result, int max_depth, int skip_count, const void* uc) = 0;
};

}  // namespace heap_memory_profiler

#endif  // SRC_COLLECT_STACK_STRACE_INL_H_
