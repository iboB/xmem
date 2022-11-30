// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once

namespace xmem {
template <typename T>
struct default_delete {
    void operator()(T* p) const noexcept {
        static_assert(sizeof(T) > 0, "default_delete of an incomplete type");
        delete p;
    }
};

template <typename T>
struct default_delete<T[]> {
    void operator()(T* ar) const noexcept {
        static_assert(sizeof(T) > 0, "default_delete of an incomplete type");
        delete[] ar;
    }
};
}