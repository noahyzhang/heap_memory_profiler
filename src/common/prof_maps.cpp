#include <stdio.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include "common/prof_maps.h"

namespace heap_memory_profiler {

static bool ExtractUntilChar(char *text, int c, char **endptr) {
    if (text == nullptr || endptr == nullptr) {
        return false;
    }
    char *found;
    found = strchr(text, c);
    if (found == NULL) {
    *endptr = NULL;
    return false;
    }

    *endptr = found;
    *found = '\0';
    return true;
}

// Increments |*text_pointer| while it points a whitespace character.
// It is to follow sscanf's whilespace handling.
static void SkipWhileWhitespace(char **text_pointer, int c) {
  if (isspace(c)) {
    while (isspace(**text_pointer) && isspace(*((*text_pointer) + 1))) {
      ++(*text_pointer);
    }
  }
}

template<class T>
static T StringToInteger(char *text, char **endptr, int base) {
    assert(false);
    return T();
}

template<>
int StringToInteger<int>(char *text, char **endptr, int base) {
    return strtol(text, endptr, base);
}

template<>
int64_t StringToInteger<int64_t>(char *text, char **endptr, int base) {
    return strtoll(text, endptr, base);
}

template<>
uint64_t StringToInteger<uint64_t>(char *text, char **endptr, int base) {
    return strtoull(text, endptr, base);
}

template<typename T>
static T StringToIntegerUntilChar(
    char *text, int base, int c, char **endptr_result) {
    CHECK_NE(endptr_result, NULL);
    *endptr_result = NULL;

    char *endptr_extract;
    if (!ExtractUntilChar(text, c, &endptr_extract))
    return 0;

    T result;
    char *endptr_strto;
    result = StringToInteger<T>(text, &endptr_strto, base);
    *endptr_extract = c;

    if (endptr_extract != endptr_strto)
    return 0;

    *endptr_result = endptr_extract;
    SkipWhileWhitespace(endptr_result, c);

    return result;
}

static char *CopyStringUntilChar(
    char *text, unsigned out_len, int c, char *out) {
    char *endptr;
    if (!ExtractUntilChar(text, c, &endptr))
    return NULL;

    strncpy(out, text, out_len);
    out[out_len-1] = '\0';
    *endptr = c;

    SkipWhileWhitespace(&endptr, c);
    return endptr;
}

template<typename T>
static bool StringToIntegerUntilCharWithCheck(
    T *outptr, char *text, int base, int c, char **endptr) {
    *outptr = StringToIntegerUntilChar<T>(*endptr, base, c, endptr);
    if (*endptr == NULL || **endptr == '\0') return false;
    ++(*endptr);
    return true;
}

static bool parse_proc_maps_line(char *text, uint64_t *start, uint64_t *end,
                              char *flags, uint64_t *offset,
                              int *major, int *minor, int64_t *inode,
                              unsigned *filename_offset) {
    char *endptr = text;
    if (endptr == NULL || *endptr == '\0')  return false;

    if (!StringToIntegerUntilCharWithCheck(start, endptr, 16, '-', &endptr))
    return false;

    if (!StringToIntegerUntilCharWithCheck(end, endptr, 16, ' ', &endptr))
    return false;

    endptr = CopyStringUntilChar(endptr, 5, ' ', flags);
    if (endptr == NULL || *endptr == '\0')  return false;
    ++endptr;

    if (!StringToIntegerUntilCharWithCheck(offset, endptr, 16, ' ', &endptr))
    return false;

    if (!StringToIntegerUntilCharWithCheck(major, endptr, 16, ':', &endptr))
    return false;

    if (!StringToIntegerUntilCharWithCheck(minor, endptr, 16, ' ', &endptr))
    return false;

    if (!StringToIntegerUntilCharWithCheck(inode, endptr, 10, ' ', &endptr))
    return false;

    *filename_offset = (endptr - text);
    return true;
}

ProcMapsIterator::ProcMapsIterator(pid_t pid, Buffer* buffer) {
    init(pid, buffer);
}

ProcMapsIterator::~ProcMapsIterator() {
    if (fd_ >= 0) {
        close(fd_);
    }
    delete dynamic_buffer_;
}

static void construct_filename(const char* spec, pid_t pid, char* buf, int buf_size) {
    snprintf(buf, buf_size, spec, static_cast<int>(pid ? pid : getpid()), buf_size);
}

void ProcMapsIterator::init(pid_t pid, Buffer* buffer) {
    pid_ = pid;
    dynamic_buffer_ = nullptr;
    if (buffer == nullptr) {
        // 一般情况下，buffer 都不为空；如果为空，则使用 new 来创建空间
        buffer = dynamic_buffer_ = new Buffer();
    } else {
        dynamic_buffer_ = nullptr;
    }
    input_buf_ = buffer->buf;
    start_text_ = end_text_ = next_line_ = input_buf_;
    end_buf_ = input_buf_ + Buffer::buf_size - 1;
    next_line_ =  input_buf_;

    if (pid == 0) {
        construct_filename("proc/self/maps", 1, input_buf_, Buffer::buf_size);
    } else {
        construct_filename("/proc/%d/maps", pid, input_buf_, Buffer::buf_size);
    }

    fd_ = open(input_buf_, O_RDONLY);
}

bool ProcMapsIterator::Next(uint64_t* start, uint64_t* end, char** flags,
    uint64_t* offset, int64_t* inode, char** filename) {
    return NextExt(start, end, flags, offset, inode, filename, NULL, NULL,
                NULL, NULL, NULL);
}

bool ProcMapsIterator::NextExt(uint64_t *start, uint64_t *end, char **flags,
    uint64_t *offset, int64_t *inode, char **filename,
    uint64_t *file_mapping, uint64_t *file_pages,
    uint64_t *anon_mapping, uint64_t *anon_pages,
    dev_t *dev) {
    do {
        start_text_ = next_line_;
        next_line_ = static_cast<char*>(memchr(start_text_, '\n', end_text_- start_text_));
        if (!next_line_) {
            int count = end_text_ - start_text_;
            memmove(input_buf_, start_text_, count);
            start_text_ = input_buf_;
            end_text_ = input_buf_ + count;

            int nread = 0;
            for (; end_text_ < end_buf_;) {
                nread = read(fd_, end_text_, end_buf_ - end_text_);
                if (nread < 0 && errno == EINTR) {
                    continue;
                }
                if (nread > 0) {
                    end_text_ += nread;
                } else {
                    break;
                }
            }
            if (end_text_ != end_buf_ && nread == 0) {
                memset(end_text_, 0, end_buf_ - end_text_);
            }
            *end_text_ = '\n';
            next_line_ = static_cast<char*>(memchr(start_text_, '\n', end_text_ + 1 - start_text_));
        }
        *next_line_ = 0;
        next_line_ += ((next_line_ < end_text_) ? 1 : 0);
        uint64_t tmp_start, tmp_end, tmp_offset;
        int64_t tmp_inode;
        int major, minor;
        unsigned filename_offset = 0;
        if (!parse_proc_maps_line(start_text_,
            start ? start : &tmp_start,
            end ? end : &tmp_end,
            flags_,
            offset ? offset : &tmp_offset,
            &major, &minor,
            inode ? inode : &tmp_inode,
            &filename_offset)) {
            continue;
        }
        size_t start_text_len = strlen(start_text_);
        if (filename_offset == 0 || filename_offset > start_text_len) {
            filename_offset = start_text_len;
        }
        if (flags) {
            *flags = flags_;
        }
        if (filename) {
            *filename = start_text_ + filename_offset;
        }
        if (dev) {
            *dev = minor | (major << 8);
        }
        return true;
    } while (end_text_ > input_buf_);
    return false;
}

int ProcMapsIterator::format_line(char* buffer, int buf_size,
    uint64_t start, uint64_t end, const char* flags,
    uint64_t offset, int64_t inode, const char* filename,
    dev_t dev) {
    // We assume 'flags' looks like 'rwxp' or 'rwx'.
    char r = (flags && flags[0] == 'r') ? 'r' : '-';
    char w = (flags && flags[0] && flags[1] == 'w') ? 'w' : '-';
    char x = (flags && flags[0] && flags[1] && flags[2] == 'x') ? 'x' : '-';
    // p always seems set on linux, so we set the default to 'p', not '-'
    char p = (flags && flags[0] && flags[1] && flags[2] && flags[3] != 'p')
        ? '-' : 'p';

    const int rc = snprintf(buffer, buf_size,
                            "%08" PRIx64 "-%08" PRIx64 " %c%c%c%c %08" PRIx64 " %02x:%02x %-11" PRId64 " %s\n",
                            start, end, r, w, x, p, offset,
                            static_cast<int>(dev/256), static_cast<int>(dev%256),
                            inode, filename);
    return (rc < 0 || rc >= buf_size) ? 0 : rc;
}



int fill_proc_self_maps(char buf[], int size, bool* wrote_all) {
    ProcMapsIterator::Buffer iter_buf;
    ProcMapsIterator it(0, &iter_buf);

    uint64_t start, end, offset;
    int64_t inode;
    char* flags, *filename;
    int bytes_written = 0;
    *wrote_all = true;
    for (; it.Next(&start, &end, &flags, &offset, &inode, &filename);) {
        const int line_len = it.format_line(
            buf + bytes_written, size - bytes_written,
            start, end, flags, offset, inode, filename, 0);
        if (line_len == 0) {
            *wrote_all = false;
        } else {
            bytes_written += line_len;
        }
    }
    return bytes_written;
}

}  // namespace heap_memory_profiler
