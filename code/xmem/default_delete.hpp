// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <type_traits>

namespace xmem {
template <typename T>
struct default_delete {
    constexpr default_delete() = default;

    template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    default_delete(const default_delete<U>&) {};

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