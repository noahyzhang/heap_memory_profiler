#include <stddef.h>
#include <gflags/gflags.h>
#include <fstream>
#include "collect/sample_config.h"
#include "collect/record_manager.h"
#include "common/spinlock.h"
#include "common/init.h"

/**
 * 注意不要在此库中主动或者被动的调用申请、释放内存的操作，可能会造成死循环，主要谨防第三方库
 * 
 */

namespace heap_memory_profiler {

DEFINE_int64(heap_profile_allocation_interval,
    1 << 30,
    "If non-zero, dump heap profiling information once every "
    "specified number of bytes allocated by the program since "
    "the last dump.");

DEFINE_int64(heap_profile_deallocation_interval,
    0,
    "If non-zero, dump heap profiling information once every "
    "specified number of bytes deallocated by the program "
    "since the last dump.");

DEFINE_int64(heap_profile_inuse_interval,
    100 << 20,
    "If non-zero, dump heap profiling information whenever "
    "the high-water memory usage mark increases by the specified "
    "number of bytes.");

DEFINE_int64(heap_profile_time_interval,
    0,
    "If non-zero, dump heap profiling information once every "
    "specified number of seconds since the last dump.");

DEFINE_bool(only_mmap_profiler,
    false,
    "If heap-profiling is on, only profile mmap, mremap, and sbrk; "
    "do not profile malloc/new/etc");


static SpinLock heap_lock;
static bool is_on = false;
static bool is_dumping = false;
static char* filename_prefix = nullptr;

static int dump_count = 0;
static int64_t last_dump_alloc_size = 0;
static int64_t last_dump_dealloc_size = 0;
static int64_t high_water_mark = 0;
static int64_t last_dump_time_s = 0;

static RecordManager* record_manager = nullptr;

static void dump_profile(const char* reason) {
    if (filename_prefix == nullptr) {
        return;
    }
    is_dumping = true;
    // 给输出文件取名字
    char filename[1024] = {0};
    dump_count++;
    snprintf(filename, sizeof(filename), "%s.%04d%s",
        filename_prefix, dump_count, SampleFixedVariable::heap_file_suffix);
    // 将内存管理信息写入文件
    std::ofstream ofstream;
    try {
        ofstream.open(filename, std::ofstream::out | std::ofstream::trunc);
    } catch (...) {
        // LOG
        return;
    }
    record_manager->fill_ordered_profile(ofstream);
    ofstream.close();
    is_dumping = false;
}

static void check_dump_profile() {
    if (is_dumping) {
        return;
    }
    const auto& total_mem_info = record_manager->get_total_mem_info();
    int64_t inuse_bytes = total_mem_info.alloc_size - total_mem_info.dealloc_size;
    bool need_to_dump = false;
    char buf[128] = {0};
    if (FLAGS_heap_profile_allocation_interval > 0
        && total_mem_info.alloc_size >= last_dump_alloc_size + FLAGS_heap_profile_allocation_interval) {
        snprintf(buf, sizeof(buf),
            "%d MB allocated cumulatively, %d MB currently in use",
            total_mem_info.alloc_size >> 20, inuse_bytes >> 20);
        need_to_dump = true;
    } else if (FLAGS_heap_profile_deallocation_interval > 0
        && total_mem_info.dealloc_size >= last_dump_alloc_size + FLAGS_heap_profile_deallocation_interval) {
        snprintf(buf, sizeof(buf),
            "%d MB deallocated cumulatively, %d MB currently in use",
            total_mem_info.dealloc_size >> 20, inuse_bytes >> 20);
        need_to_dump = true;
    } else if (FLAGS_heap_profile_inuse_interval > 0
        && inuse_bytes > high_water_mark + FLAGS_heap_profile_inuse_interval) {
        snprintf(buf, sizeof(buf),
            "%d MB currently in use",
            inuse_bytes >> 20);
        need_to_dump = true;
    } else if (FLAGS_heap_profile_time_interval > 0) {
        int64_t current_time_s = time(nullptr);
        if (current_time_s - last_dump_time_s >= FLAGS_heap_profile_time_interval) {
            snprintf(buf, sizeof(buf), "%d sec since the last dump", current_time_s - last_dump_time_s);
            need_to_dump = true;
            last_dump_time_s = current_time_s;
        }
    }
    if (need_to_dump) {
        dump_profile(buf);
        last_dump_alloc_size = total_mem_info.alloc_size;
        last_dump_dealloc_size = total_mem_info.dealloc_size;
        if (inuse_bytes > high_water_mark) {
            high_water_mark = inuse_bytes;
        }
    }
}

/**
 * @brief 收集内存的申请信息
 * 
 * @param ptr 
 * @param bytes 
 * @param skip_count 
 */
static void record_alloc(const void* ptr, size_t bytes, int skip_count) {
    const int max_stack_depth = SampleConfig::get_instance().get_max_stack_depth();
    void* stack[max_stack_depth] = {0};
    int depth = RecordManager::get_caller_stack_trace(skip_count + 1, stack);
    heap_lock.lock();
    if (is_on) {
        record_manager->record_alloc(ptr, bytes, depth, stack);
        check_dump_profile();
    }
    heap_lock.unlock();
}

/**
 * @brief 收集内存的释放信息
 * 
 * @param ptr 
 */
static void record_dealloc(const void* ptr) {
    heap_lock.lock();
    if (is_on) {
        record_manager->record_dealloc(ptr);
        check_dump_profile();
    }
    heap_lock.unlock();
}

extern "C" void heap_profiler_start(const char* prefix) {
    LockGuard<SpinLock> lock_guard(&heap_lock);

    if (is_on) {
        return;
    }
    is_on = true;


    last_dump_alloc_size = 0;
    last_dump_dealloc_size = 0;
    high_water_mark = 0;
    last_dump_time_s = 0;

    if (FLAGS_only_mmap_profiler == false) {
        
    }
}

static void heap_profiler_init() {

}

static void heap_profiler_destory() {

}

REGISTER_MODULE_INITIALIZER(heap_profiler, heap_profiler_init(), heap_profiler_destory());


}  // namespace heap_memory_profiler
