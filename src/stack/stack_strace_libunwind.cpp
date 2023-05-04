#include <libunwind.h>
#include <string.h>
#include "stack_strace.h"

namespace heap_memory_profiler {

#if defined(PROFILER_ENABLE_UNWIND_FROM_UCONTEXT)
    && (defined(__i386__) || defined(__x86_64__)) && defined(__GNU_LIBRARY__)
#define STACKTRACE_UNWIND_CONTEXT_IS_UCONTEXT 1
#endif

static __thread int recusive;

class CollectStackLibunwindImpl : public CollectStackBase {
public:
    int get_stack_trace(void** result, int max_depth, int skip_count) override;

    int get_stack_trace_with_context(void** result, int max_depth,
        int skip_count, const void* uc) override;
};

int CollectStackLibunwindImpl::get_stack_trace(void** result, int max_depth, int skip_count) {
    if (recusive) {
        return 0;
    }
    ++recusive;
#if (IS_WITH_CONTEXT && defined(STACKTRACE_UNWIND_CONTEXT_IS_UCONTEXT))

#else
#endif
}

int CollectStackLibunwindImpl::get_stack_trace_with_context(
    void** result, int max_depth,
    int skip_count, const void* uc) {

}

}  // namespace heap_memory_profiler
