#include <stddef.h>
#include <string.h>
#include <gflags/gflags.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "sample_config.h"
#include "record_manager.h"
#include "common/spinlock.h"
#include "common/init.h"
#include "malloc_hook.h"
#include "common/private_memory_manager.h"

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

// DEFINE_bool(only_mmap_profiler,
//     false,
//     "If heap-profiling is on, only profile mmap, mremap, and sbrk; "
//     "do not profile malloc/new/etc");


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

static char* global_profiler_buffer = nullptr;

static void* profiler_alloc(size_t bytes) {
    return PrivateMemoryManager::get_instance().alloc(bytes);
}

static void profiler_dealloc(void* ptr) {
    PrivateMemoryManager::get_instance().dealloc(ptr);
}

static char* do_get_heap_profile(char* buf, int buf_len) {
    if (buf == nullptr || buf_len < 1) {
        return nullptr;
    }
    int bytes_written = 0;
    if (is_on) {
        SampleStats stat = record_manager->get_total_mem_info();
        bytes_written = record_manager->fill_ordered_profile(buf, buf_len-1);
        if (!stat.is_equivalent(record_manager->get_total_mem_info())) {
            // TODO
            return nullptr;
        }
    }
    if (bytes_written < 0 || bytes_written >= buf_len) {
        return nullptr;
    }
    buf[bytes_written] = '\0';
    if (strlen(buf) != bytes_written) {
        // TODO
        return nullptr;
    }
    return buf;
}

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
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0664);
    if (fd < 0) {
        // TODO
        is_dumping = false;
        return;
    }
    if (global_profiler_buffer == nullptr) {
        global_profiler_buffer = reinterpret_cast<char*>(
            profiler_alloc(SampleFixedVariable::profile_buffer_size));
    }
    char* profile_data = do_get_heap_profile(global_profiler_buffer, SampleFixedVariable::profile_buffer_size);
    if (profile_data == nullptr) {
        close(fd);
        is_dumping = false;
        return;
    }
    int profile_data_len = strlen(profile_data);
    for (; profile_data_len > 0;) {
        ssize_t count = write(fd, profile_data, profile_data_len);
        if (count < 0 && errno == EINTR) {
            continue;
        }
        if (count <= 0) {
            break;
        }
        profile_data += count;
        profile_data_len -= count;
    }
    close(fd);

    // record_manager->fill_ordered_profile(ofstream);
    // ofstream.close();
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
    void* stack[SampleFixedVariable::max_stack_depth] = {0};
    int depth = RecordManager::get_caller_stack_trace(skip_count + 1, stack);
    LockGuard<SpinLock> guard(&heap_lock);
    if (is_on) {
        record_manager->record_alloc(ptr, bytes, depth, stack);
        check_dump_profile();
    }
}

/**
 * @brief 收集内存的释放信息
 * 
 * @param ptr 
 */
static void record_dealloc(const void* ptr) {
    LockGuard<SpinLock> guard(&heap_lock);
    if (is_on) {
        record_manager->record_dealloc(ptr);
        check_dump_profile();
    }
}

void alloc_hook(const void* ptr, size_t size) {
    if (ptr != nullptr) {
        record_alloc(ptr, size, 0);
    }
}

void dealloc_hook(const void* ptr) {
    if (ptr != nullptr) {
        record_dealloc(ptr);
    }
}

extern "C" int heap_profiler_start(const char* prefix) {
    LockGuard<SpinLock> lock_guard(&heap_lock);
    if (is_on) {
        return;
    }
    is_on = true;

    global_profiler_buffer = reinterpret_cast<char*>(profiler_alloc(SampleFixedVariable::profile_buffer_size));
    if (!global_profiler_buffer) {
        return -1;
    }
    void* tmp_mem_buf = profiler_alloc(sizeof(RecordManager));
    if (!tmp_mem_buf) {
        return -2;
    }
    record_manager = new(tmp_mem_buf) RecordManager(profiler_alloc, profiler_dealloc);
    if (!record_manager) {
        profiler_dealloc(tmp_mem_buf);
        return -3;
    }

    last_dump_alloc_size = 0;
    last_dump_dealloc_size = 0;
    high_water_mark = 0;
    last_dump_time_s = 0;

    MallocHook::add_alloc_hook(&alloc_hook);
    MallocHook::add_dealloc_hook(&dealloc_hook);

    const int prefix_len = strlen(prefix);
    filename_prefix = reinterpret_cast<char*>(profiler_alloc(prefix_len + 1));
    memcpy(filename_prefix, prefix, prefix_len);
    filename_prefix[prefix_len] = '\0';

    return 0;
}

extern "C" int heap_profiler_stop() {
    LockGuard<SpinLock> lock_guard(&heap_lock);
    if (!is_on) {
        return -1;
    }
    MallocHook::remove_alloc_hook(&alloc_hook);
    MallocHook::remove_dealloc_hook(&dealloc_hook);

    record_manager->~RecordManager();
    profiler_dealloc(record_manager);
    record_manager = nullptr;

    profiler_dealloc(global_profiler_buffer);
    global_profiler_buffer = nullptr;

    profiler_dealloc(filename_prefix);
    filename_prefix = nullptr;

    is_on = false;
    return 0;
}

static void heap_profiler_init() {
}

static void heap_profiler_destory() {

}

REGISTER_MODULE_INITIALIZER(heap_profiler, heap_profiler_init(), heap_profiler_destory());


}  // namespace heap_memory_profiler
