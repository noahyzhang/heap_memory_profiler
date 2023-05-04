#include <fstream>
#include "util.h"

namespace heap_memory_profiler {

std::string Util::get_proc_self_maps() {
    std::ifstream maps_file("/proc/self/maps");
    std::string line;
    std::string maps_content;
    for (; std::getline(maps_file, line); ) {
        maps_content += line + "\n";
    }
    return maps_content;
}

int Util::get_caller_stack_trace(void** result, int max_depth, int skip_count) {
    static const int max_skip = 32 + 6 + 3;
    static const int stack_size = max_skip + 1;
    void* stack[stack_size];
    int depth = get_stack_trace(stack, stack_size, 1);
}

}  // namespace heap_memory_profiler

