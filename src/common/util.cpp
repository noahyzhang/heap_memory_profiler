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

}  // namespace heap_memory_profiler

