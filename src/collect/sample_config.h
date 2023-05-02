/**
 * @file sample_config.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-05-02
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

namespace heap_memory_profiler {

class SampleConfig {
public:
    static SampleConfig& get_instance() {
        static SampleConfig instance;
        return instance;
    }

public:
    int get_max_stack_depth() {
        return max_stack_depth_;
    }

private:
    int max_stack_depth_;
};

class SampleFixedVariable {
public:
    static const int hash_table_size;
    static const char* heap_file_suffix;
    static const char* profile_header;
    static const char* proc_self_maps_header;
};

}  // namespace heap_memory_profiler
