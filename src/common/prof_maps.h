/**
 * @file prof_maps.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-05-09
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef SRC_COMMON_PROF_MAPS_H_
#define SRC_COMMON_PROF_MAPS_H_

#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <sys/types.h>

namespace heap_memory_profiler {

class ProcMapsIterator {
public:
    struct Buffer {
        static const size_t buf_size = PATH_MAX + 1024;
        char buf[buf_size];
    };

public:
    ProcMapsIterator(pid_t pid, Buffer* buffer);
    ~ProcMapsIterator();

private:
    void init(pid_t pid, Buffer* buffer);

public:
    static int format_line(char* buffer, int buf_size,
        uint64_t start, uint64_t end, const char* flags,
        uint64_t offset, int64_t inode, const char* filename,
        dev_t dev);

    bool Next(uint64_t* start, uint64_t* end, char** flags,
        uint64_t* offset, int64_t* inode, char** filename);

private:
  bool NextExt(uint64_t *start, uint64_t *end, char **flags,
               uint64_t *offset, int64_t *inode, char **filename,
               uint64_t *file_mapping, uint64_t *file_pages,
               uint64_t *anon_mapping, uint64_t *anon_pages,
               dev_t *dev);

private:
    char* input_buf_;
    char* start_text_;
    char* end_text_;
    char* next_line_;
    char* end_buf_;

    int fd_;
    pid_t pid_;
    char flags_[10];
    Buffer* dynamic_buffer_;
    bool using_maps_backing_;
};

int fill_proc_self_maps(char buf[], int size, bool* wrote_all);

}  // namespace heap_memory_profiler

#endif  // SRC_COMMON_PROF_MAPS_H_
