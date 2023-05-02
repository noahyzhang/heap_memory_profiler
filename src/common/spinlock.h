/**
 * @file spinlock.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-05-02
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include <atomic>
#include <thread>

namespace heap_memory_profiler {

class SpinLock {
public:
    SpinLock() : flag_(false) {}
    ~SpinLock() = default;
    SpinLock(const SpinLock&) = delete;
    SpinLock& operator=(const SpinLock&) = delete;

    /**
     * @brief 加锁
     * 
     */
    void lock() {
        bool expect = false;
        while (!flag_.compare_exchange_weak(expect, true)) {
            // 这里必须将 expect 复原，执行失败时 expect 结果是未定的
            expect = false;
            // 让出 cpu
            std::this_thread::yield();
        }
    }

    /**
     * @brief 解锁
     * 
     */
    void unlock() {
        flag_.store(false);
    }

private:
    // 用于实现自旋锁的原子变量
    std::atomic<bool> flag_;
};

template <typename T>
class LockGuard {
public:
    explicit LockGuard(T* loc) : loc_(loc) {
        loc_->lock();
    }
    ~LockGuard() {
        loc_->unlock();
    }

private:
    T* loc_;
};

}  // namespace heap_memory_profiler
