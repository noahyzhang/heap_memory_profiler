#include "stack_strace_inl.h"
#include "stack_strace.h"

namespace heap_memory_profiler {

#if defined(HAVA_GST_instrument)
static CollectStackBase* collect_stack_impl = &impl__instrument;
#elif defined(HAVA_GST_win32)
static GetStackImplementation *get_stack_impl = &impl__win32;
#elif defined(HAVE_GST_generic_fp) && !defined(NO_FRAME_POINTER) \
   && !defined(__riscv) \
   && (!defined(HAVE_GST_libunwind) || defined(TCMALLOC_DONT_PREFER_LIBUNWIND))
static GetStackImplementation *get_stack_impl = &impl__generic_fp;
#elif defined(HAVE_GST_x86) && defined(TCMALLOC_DONT_PREFER_LIBUNWIND)
static GetStackImplementation *get_stack_impl = &impl__x86;
#elif defined(HAVE_GST_ppc) && defined(TCMALLOC_DONT_PREFER_LIBUNWIND)
static GetStackImplementation *get_stack_impl = &impl__ppc;
#elif defined(HAVE_GST_libunwind)
static GetStackImplementation *get_stack_impl = &impl__libunwind;
#elif defined(HAVE_GST_libgcc)
static GetStackImplementation *get_stack_impl = &impl__libgcc;
#elif defined(HAVE_GST_generic)
static GetStackImplementation *get_stack_impl = &impl__generic;
#elif defined(HAVE_GST_arm)
static GetStackImplementation *get_stack_impl = &impl__arm;
#elif 0
// This is for the benefit of code analysis tools that may have
// trouble with the computed #include above.
# include "stacktrace_x86-inl.h"
# include "stacktrace_libunwind-inl.h"
# include "stacktrace_generic-inl.h"
# include "stacktrace_powerpc-inl.h"
# include "stacktrace_win32-inl.h"
# include "stacktrace_arm-inl.h"
# include "stacktrace_instrument-inl.h"
#else
#error Cannot calculate stack trace: will need to write for your environment
#endif

static int frame_forcer(int rv) {
    return rv;
}

int get_stack_frames(void** result, int* sizes, int max_depth, int skip_count) {
    return frame_forcer()
}

int get_stack_frames_with_context(void** result, int* size, int max_depth, int skip_count, const void* uc) {

}

int get_stack_strace(void** result, int max_depth, int skip_count) {

}

int get_stack_trace_with_context(void** result, int max_depth, int skip_count, const void* uc) {

}

}  // namespace heap_memory_profiler
