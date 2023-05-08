/**
 * @file address_map.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-05-08
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef SRC_COMMON_ADDRESS_MAP_H_
#define SRC_COMMON_ADDRESS_MAP_H_

namespace heap_memory_profiler {

template <typename Value>
class AddressMap {
public:
    using Allocator = void*(*)(size_t);
    using Deallocator = void(*)(void*);
    using Key = const void*;

    AddressMap(Allocator alloc, Deallocator dealloc);
    ~AddressMap();

public:
    inline const Value* find(Key key) const;
    inline Value* find_mutable(Key key);
    void insert(Key key, Value value);
    bool find_and_remove(Key key, Value* removed_value);

private:
    using Number = uintptr_t;
    static const int block_bits = 7;
    static const int block_size = 1 << block_bits;

    struct Entry {
        Entry* next;
        Key key;
        Value value;
    };
    static const int cluster_bits = 13;

private:
    Allocator alloc_;
    Deallocator dealloc_;

};

}  // namespace heap_memory_profiler

#endif  // SRC_COMMON_ADDRESS_MAP_H_
