/**
 * @file init.h
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

class Initializer {
public:
    using VoidFunction = void(*)(void);

    Initializer(const char* name, VoidFunction ctor, VoidFunction dtor)
        : name_(name), destructor_(dtor) {
        if (ctor) {
            ctor();
        }
    }
    ~Initializer() {
        if (destructor_) {
            destructor_();
        }
    }

private:
    const char* const name_;
    const VoidFunction destructor_;
};

// 实际的注册
#define REGISTER_MODULE_MODULE_UNIQUE_INTERNAL(name, ctor, dtor, UniqueID)  \
    namespace {                                                             \
        static void init_module_##UniqueID() { ctor; }               \
        static void destruct_module_##UniqueID() { dtor; }            \
        Initializer init_module_##UniqueID(                          \
            #name,                                                          \
            init_module_##UniqueID,                                  \
            destruct_module_##UniqueID);                              \
    }

// 增加中间层，为了保证 __COUNTER__ 可用
#define REGISTER_MODULE_MODULE_UNIQUE(name, ctor, dtor, UniqueID)  \
    REGISTER_MODULE_MODULE_UNIQUE_INTERNAL(name, ctor, dtor, UniqueID)

// 用于注册需要提前初始化的模块
#define REGISTER_MODULE_INITIALIZER(name, ctor, dtor)  \
    REGISTER_MODULE_MODULE_UNIQUE(name, ctor, dtor, __COUNTER__)

}  // namespace heap_memory_profiler
