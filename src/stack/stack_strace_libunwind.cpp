#include <libunwind.h>
#include <string.h>
#include "stack_strace_inl.h"

namespace heap_memory_profiler {

#if defined(PROFILER_ENABLE_UNWIND_FROM_UCONTEXT)
    && (defined(__i386__) || defined(__x86_64__)) && defined(__GNU_LIBRARY__)
#define STACKTRACE_UNWIND_CONTEXT_IS_UCONTEXT 1
#endif

static __thread int recusive;

class CollectStackLibunwindImpl : public CollectStackBase {
public:
    int get_stack_frames(
        void** result, int* size, int max_depth, int skip_count) override;

    int get_stack_frames_with_context(
        void** result, int* size, int max_depth, int skip_count, const void* uc) override;

    int get_stack_trace(void** result, int max_depth, int skip_count) override;

    int get_stack_trace_with_context(void** result, int max_depth,
        int skip_count, const void* uc) override;
};

int CollectStackLibunwindImpl::get_stack_trace(void** result, int max_depth, int skip_count) {
    if (recusive) {
        return 0;
    }
    ++recusive;
    unw_context_t uc;
    unw_getcontext(&uc);
    skip_count += 2;
    unw_cursor_t cursor;
    if (unw_init_local(&cursor, &uc) < 0) {
        return -1;
    }
    for (; skip_count--; ) {
        if (unw_step(&cursor) <= 0) {
            goto out;
        }
    }
    int n = 0;
    void* ip;
    for (; n < max_depth; ) {
        if (unw_get_reg(&cursor, UNW_REG_IP, (unw_word_t*)&ip) < 0) {
            break;
        }
        result[n++] = ip;
        if (unw_step(&cursor) <= 0) {
            break;
        }
    }
out:
    --recusive;
    return n;
}

int CollectStackLibunwindImpl::get_stack_trace_with_context(
    void** result, int max_depth,
    int skip_count, const void* uc) {

}

}  // namespace heap_memory_profiler
