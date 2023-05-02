cmake_minimum_required(VERSION 2.8)

project(heap_memory_profiler)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Werror -Wextra -fPIC -Wno-error=unused-parameter -fno-omit-frame-pointer")

include_directories(
    src
)

file(GLOB_RECURSE SRCS
    src/*.cpp
)

add_library(heap_memory_profiler SHARED ${SRCS})

target_link_libraries(heap_memory_profiler
    pthread
    gflags
)
