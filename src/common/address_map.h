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
    static int hash_int(Number x) {
        const uint32_t m = static_cast<uint32_t>(x) * hash_multiplier_;
        return static_cast<int>(m >> (32 - hash_bits_));
    }

    Cluster* find_cluster(Number address, bool is_create) {
        const Number cluster_id = address >> (block_bits_ + cluster_bits_);
        const int h = hash_int(cluster_id);
        for (Cluster* c = hash_table_[h]; c != nullptr; c = c->next) {
            if (c->id == cluster_id) {
                return c;
            }
        }
        if (is_create) {
            Cluster* c = New(Cluster)(1);
            c->id = cluster_id;
            c->next = hash_table_[h];
            hash_table_[h] = c;
            return c;
        }
        return nullptr;
    }

    static int block_id(Number address) {
        return (address >> block_bits_) & (cluster_blocks_ - 1);
    }

    template <typename T>
    T* New(int num) {
        void* ptr = (*alloc_)(sizeof(Object) + num * sizeof(T));
        memset(ptr, 0, sizeof(Object) + num * sizeof(T));
        Object* obj = reinterpret_cast<Object*>(ptr);
        obj->next = allocated_;
        allocated_ = obj;
        return reinterpret_cast<T*>(reinterpret_cast<Object*>(ptr) + 1);
    }

private:
    using Number = uintptr_t;
    static const int block_bits_ = 7;
    static const int block_size_ = 1 << block_bits_;

    static const int cluster_bits_ = 13;
    static const Number cluster_size_ = 1 << (block_bits_ + cluster_bits_);
    static const int cluster_blocks_ = 1 << cluster_bits_;

    static const int hash_bits_ = 12;
    static const int hash_size_ = 1 << hash_bits_;

    static const int ALLOC_COUNT_ = 64;

    static const uint32_t hash_multiplier_ = 2654435769u;

private:
    struct Entry {
        Entry* next;
        Key key;
        Value value;
    };

    struct Cluster {
        Cluster* next;
        Number id;
        Entry* blocks[cluster_blocks_];
    };

    struct Object {
        Object* next;
    };

private:
    Allocator alloc_;
    Deallocator dealloc_;
    Object* allocated_;

    Cluster** hash_table_;
    Entry* free_;
};

template <typename Value>
AddressMap<Value>::AddressMap(Allocator alloc, Deallocator dealloc)
    : free_(nullptr)
    , alloc_(alloc)
    , dealloc_(dealloc)
    , deallocated_(nullptr) {
    hash_table_ = New<Cluster*>(hash_size_);
}

template <typename Value>
AddressMap<Value>::~AddressMap() {
    for (Object* obj = allocated_; obj != nullptr;) {
        Object* next = obj->next;
        (*dealloc_)(obj);
        obj = next;
    }
}

template <typename Value>
inline const Value* AddressMap<Value>::find(Key key) const {
    return const_cast<AddressMap*>(this)->find_mutable(key);
}

template <typename Value>
inline Value* AddressMap<Value>::find_mutable(Key key) {
    const Number num = reinterpret_cast<Number>(key);
    const Cluster* const c = find_cluster(num, false);
    if (c != nullptr) {
        for (Entry* e = c->blocks[block_id(num)]; e != nullptr; e = e->next) {
            if (e->key == key) {
                return &e->value;
            }
        }
    }
    return nullptr;
}

template <typename Value>
void AddressMap<Value>::insert(Key key, Value value) {
    const Number num = reinterpret_cast<Number>(key);
    Cluster* const c = find_cluster(num, true);
    const int block = block_id(num);
    for (Entry* e = c->blocks[block]; e != nullptr; e = e->next) {
        if (e->key == key) {
            e->value = value;
            return;
        }
    }
    if (free_ == nullptr) {
        Entry* array = New<Entry>(ALLOC_COUNT_);
        for (int i = 0; i < ALLOC_COUNT_; ++i) {
            array[i].next = &array[i+1];
        }
        array[ALLOC_COUNT_ - 1].next = free_;
        free_ = &array[0]; 
    }
    Entry* e = free_;
    free_ = e->next;
    e->key = key;
    e->value = value;
    e->next = c->blocks[block];
    c->blocks[block] = e;
}

template <typename Value>
bool AddressMap<Value>::find_and_remove(Key key, Value* removed_value) {
  const Number num = reinterpret_cast<Number>(key);
  Cluster* const c = find_cluster(num, false);
  if (c != NULL) {
    for (Entry** p = &c->blocks[block_id(num)]; *p != NULL; p = &(*p)->next) {
      Entry* e = *p;
      if (e->key == key) {
        *removed_value = e->value;
        *p = e->next;
        e->next = free_;
        free_ = e;
        return true;
      }
    }
  }
  return false;
}

}  // namespace heap_memory_profiler

#endif  // SRC_COMMON_ADDRESS_MAP_H_
