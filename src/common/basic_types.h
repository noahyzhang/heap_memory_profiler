/**
 * @file basic_types.h
 * @auther noahyzhang
 * @brief
 * @version 0.1
 * @date 2023/5/3
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef SRC_COMMON_BASIC_TYPES_H_
#define SRC_COMMON_BASIC_TYPES_H_

namespace heap_memory_profiler {

template<typename Dest, typename Source>
inline Dest bit_cast(const Source &source) {
    static_assert(sizeof(Dest) == sizeof(Source), "bit_cast unequal size");
    Dest dest;
    memcpy(&dest, &source, sizeof(dest));
    return dest;
}

}  // namespace heap_memory_profiler

#endif  // SRC_COMMON_BASIC_TYPES_H_
