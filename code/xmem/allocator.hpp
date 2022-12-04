// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <memory>

namespace xmem {

// a bare-bones allocator which is enough for the needs of shared pointers
template <typename T>
class allocator {
public:
    using value_type = T;

    allocator() noexcept = default;
    allocator(const allocator&) noexcept = default;
    template <typename U>
    allocator(const allocator<U>&) noexcept {}

    static inline constexpr std::align_val_t align_val{alignof(T)};

    [[nodiscard]] T* allocate(size_t n) {
        auto ret = ::operator new(n * sizeof(T), align_val);
        return reinterpret_cast<T*>(ret);
    }
    void deallocate(T* ptr, size_t) {
        auto del = reinterpret_cast<void*>(ptr);
        ::operator delete(del, align_val);
    }
};

}
