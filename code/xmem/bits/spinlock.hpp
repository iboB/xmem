// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <atomic>
#include <new>

namespace xmem::impl {

struct spinlock {
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
    void lock() noexcept {
        while (flag.test_and_set(std::memory_order_acquire)) /* spin */;
    }
    void unlock() noexcept {
        flag.clear(std::memory_order_release);
    }

    struct lock_guard {
        lock_guard(spinlock& sl) noexcept : m_sl(sl) { m_sl.lock(); }
        ~lock_guard() { m_sl.unlock(); }
    private:
        spinlock& m_sl;
    };
};

inline constexpr size_t cache_line_size =
#if defined(__cpp_lib_hardware_interference_size)
    std::hardware_destructive_interference_size
#else
    64
#endif
;

}
