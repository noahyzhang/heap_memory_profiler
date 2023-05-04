/**
 * @file util.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-05-02
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include <string>

namespace heap_memory_profiler {

class Util {
public:
    static std::string get_proc_self_maps();

    static int get_caller_stack_trace(void** result, int max_depth, int skip_count);

};

}  // namespace heap_memory_profiler
