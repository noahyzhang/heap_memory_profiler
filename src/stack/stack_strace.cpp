#include "stack/stack_strace_inl.h"
#include "stack/stack_strace_libunwind.h"
#include "stack/stack_strace.h"

namespace heap_memory_profiler {

static CollectStackLibunwindImpl stack_libunwind_impl;
static CollectStackBase *get_stack_impl = &stack_libunwind_impl;

static int frame_forcer(int rv) {
    return rv;
}

int get_stack_frames(void** result, int* sizes, int max_depth, int skip_count) {
    return frame_forcer(get_stack_impl->get_stack_frames(result, sizes, max_depth, skip_count));
}

int get_stack_frames_with_context(void** result, int* sizes, int max_depth, int skip_count, const void* uc) {
    return frame_forcer(get_stack_impl->get_stack_frames_with_context(result, sizes, max_depth, skip_count, uc));
}

int get_stack_strace(void** result, int max_depth, int skip_count) {
    return frame_forcer(get_stack_impl->get_stack_trace(result, max_depth, skip_count));
}

int get_stack_trace_with_context(void** result, int max_depth, int skip_count, const void* uc) {
    return frame_forcer(get_stack_impl->get_stack_trace_with_context(result, max_depth, skip_count, uc));
}

}  // namespace heap_memory_profiler
