// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <memory>

namespace xmem {

// a bare-bones allocator which is enough for the needs of shared pointers
template <typename T>
class allocator {
    [[nodiscard]] T* allocate(size_t n) {
        auto ret = ::operator new(std::align_val_t{alignof(T)}) char[n * sizeof(T)];
        return reinterpret_cast<T*>(ret);
    }
    void deallocate(T* ptr, size_t) {
        auto del = reinterpret_cast<char*>(ptr);
        delete[] del;
    }
};

}
