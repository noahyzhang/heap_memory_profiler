#include "sample_config.h"

namespace heap_memory_profiler {

const int SampleFixedVariable::hash_table_size = 179999;
const char* SampleFixedVariable::heap_file_suffix = ".heap";
const char* SampleFixedVariable::profile_header = "heap profile: ";
const char* SampleFixedVariable::proc_self_maps_header = "\nMAPPED_LIBRARIES:\n";
const int SampleFixedVariable::strip_frames = 3;
const int SampleFixedVariable::profile_buffer_size = 1 << 20;
const int SampleFixedVariable::max_stack_depth = 32;

}  // namespace heap_memory_profiler
