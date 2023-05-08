#include <string.h>
#include <algorithm>
#include "common/util.h"
#include "sample_config.h"
#include "stack/stack_strace.h"
#include "record_manager.h"

namespace heap_memory_profiler {

RecordManager::RecordManager(Allocator alloc, Deallocator dealloc)
    : alloc_(alloc)
    , dealloc_(dealloc) {
    

}

RecordManager::~RecordManager() {

}

void RecordManager::record_alloc(
    const void* ptr, size_t bytes,
    int stack_depth, const void* const call_stack[]) {
    SampleBucket* bucket = get_bucket(stack_depth, call_stack);
    bucket->call_alloc_num++;
    bucket->alloc_size += bytes;
    // 总体的内存管理信息
    total_mem_info_.call_alloc_num++;
    total_mem_info_.alloc_size += bytes;

    AllocValue val;
    val.set_bucket(bucket);
    val.set_alloc_bytes(bytes);
    address_mp_[ptr] = val;
}

void RecordManager::record_dealloc(const void* ptr) {
    auto iter = address_mp_.find(ptr);
    if (iter != address_mp_.end()) {
        SampleBucket* bucket = iter->second.get_bucket();
        bucket->call_dealloc_num++;
        bucket->dealloc_size += iter->second.get_alloc_bytes();
        // 总体的内存管理信息
        total_mem_info_.call_dealloc_num++;
        total_mem_info_.dealloc_size += iter->second.get_alloc_bytes();
        // 移除这个指针
        address_mp_.erase(iter);
    }
}

int RecordManager::fill_ordered_profile(char buf[], int size) {
    int map_len = snprintf(buf, size, "%s", SampleFixedVariable::proc_self_maps_header);
    if (map_len < 0 || map_len >= size) {
        return -1;
    }
    bool dummy;
    map_len += fill_proc_self_maps(buf + map_len, size - map_len, &dummy);
    if (map_len > size) {
        return -2;
    }
    char* const map_start = buf + size - map_len;
    memmove(map_start, buf, map_len);
    size -= map_len;

    SampleStats stats;
    memset(&stats, 0, sizeof(stats));
    int bucket_len = snprintf(buf, size, "%s", SampleFixedVariable::profile_header);
    if (bucket_len < 0 || bucket_len >= size) {
        return -3;
    }
    bucket_len = parse_bucket(total_mem_info_, buf, bucket_len, size, " heapprofile", &stats);

    SampleBucket** list = make_sorted_bucket_list();
    for (int i = 0; i < bucket_num_; ++i) {
        bucket_len = parse_bucket(*list[i], buf, bucket_len, size, "", &stats);
    }
    if (bucket_len >= size) {
        return -4;
    }
    dealloc_(list);
    if (buf+bucket_len > map_start) {
        return -5;
    }
    memmove(buf+bucket_len, map_start, map_len);
    return bucket_len + map_len;
}

int RecordManager::get_caller_stack_trace(int skip_count, void** stack) {
    get_stack_strace(stack, SampleBucket::max_stack_depth, SampleFixedVariable::strip_frames + skip_count + 1);
}

SampleBucket* RecordManager::get_bucket(int depth, const void* const key[]) {
    // 计算哈希
    uintptr_t hash = 0;
    for (int i = 0; i < depth; ++i) {
        hash += reinterpret_cast<uintptr_t>(key[i]);
        hash += hash << 10;
        hash ^= hash >> 6;
    }
    hash += hash << 3;
    hash ^= hash >> 11;
    // 寻找是否已经存在 bucket
    uint64_t idx = ((uint64_t)hash) % SampleFixedVariable::hash_table_size;
    SampleBucket* bucket = bucket_table_[idx];
    for (; bucket != nullptr; bucket = bucket->next) {
        if ((bucket->hash == hash)
            && (bucket->depth == depth)
            && std::equal(key, key+depth, bucket->stack)) {
            return bucket;
        }
    }
    // 创建新的 bucket
    const size_t key_size = sizeof(key[0]) * depth;
    const void** key_copy = reinterpret_cast<void**>(alloc_(key_size));
    std::copy(key, key + depth, key_copy);
    SampleBucket* new_bucket = reinterpret_cast<SampleBucket*>(alloc_(sizeof(SampleBucket)));
    memset(new_bucket, 0, sizeof(*new_bucket));
    new_bucket->hash = hash;
    new_bucket->depth = depth;
    new_bucket->stack = key_copy;
    // 头插
    new_bucket->next = bucket_table_[idx];
    bucket_table_[idx] = new_bucket;
    bucket_num_++;
    return new_bucket;
}

// std::string RecordManager::parse_bucket(const SampleBucket& bucket, const char* extra) {
//     std::string res;
//     char buf[512] = {0};
//     int num = snprintf(buf, sizeof(buf), "%6d: %8lld [%6d: %8lld ] @%s",
//         bucket.call_alloc_num - bucket.call_dealloc_num,
//         bucket.alloc_size - bucket.dealloc_size,
//         bucket.call_alloc_num,
//         bucket.alloc_size,
//         extra);
//     if (num < 0) {
//         return "";
//     }
//     buf[num] = '\0';
//     res += buf;
//     for (int i = 0; i < bucket.depth; ++i) {
//         num = snprintf(buf, sizeof(buf), " 0x%08lu", reinterpret_cast<uintptr_t>(bucket.stack[i]));
//         if (num < 0) {
//             return "";
//         }
//         buf[num] = '\0';
//         res += buf;
//     }
//     res += '\n';
//     return res;
// }

int RecordManager::parse_bucket(
    const SampleBucket& bucket, char* buf, int buf_len, int buf_size,
    const char* extra, SampleStats* profile_stats) {
    if (profile_stats != nullptr) {
        profile_stats->call_alloc_num += bucket.call_alloc_num;
        profile_stats->call_dealloc_num += bucket.call_dealloc_num;
        profile_stats->alloc_size += bucket.alloc_size;
        profile_stats->dealloc_size += bucket.dealloc_size;
    }
    int num = snprintf(buf, sizeof(buf), "%6d: %8lld [%6d: %8lld ] @%s",
        bucket.call_alloc_num - bucket.call_dealloc_num,
        bucket.alloc_size - bucket.dealloc_size,
        bucket.call_alloc_num,
        bucket.alloc_size,
        extra);
    if (num < 0 || num >= buf_size - buf_len) {
        return buf_len;
    }
    buf_len += num;
    for (int i = 0; i < bucket.depth; i++) {
        num = snprintf(buf, sizeof(buf), " 0x%08lu", reinterpret_cast<uintptr_t>(bucket.stack[i]));
        if (num < 0 || num >= buf_size - buf_len) {
            return buf_len;
        }
        buf_len += num;
    }
    num = snprintf(buf + buf_len, buf_size - buf_len, "\n");
    if (num < 0 || num >= buf_size - buf_len) {
        return buf_len;
    }
    buf_len += num;
    return buf_len;
}

SampleBucket** RecordManager::make_sorted_bucket_list() const {
    SampleBucket** list = static_cast<SampleBucket**>(alloc_(sizeof(SampleBucket) * bucket_num_));
    int bucket_count = 0;
    for (int i = 0; i < SampleFixedVariable::hash_table_size; ++i) {
        SampleBucket* curr = bucket_table_[i];
        for (; curr != nullptr; curr = curr->next) {
            list[bucket_count++] = curr;
        }
    }
    std::sort(list, list + bucket_num_, [](const SampleStats* left, const SampleStats* right) {
        return (left->alloc_size - left->dealloc_size) > (right->alloc_size - right->dealloc_size);
    });
    return list;
}

}  // namespace heap_memory_profiler
