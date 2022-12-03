// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once

namespace xmem {

// data for weak and shared pointers
template <typename CB, typename T>
struct cb_ptr_pair {
    cb_ptr_pair() noexcept = default;

    cb_ptr_pair(CB* cb, T* ptr) noexcept : cb(cb), ptr(ptr) {}

    explicit cb_ptr_pair(std::nullptr_t) : cb(nullptr), ptr(nullptr) {}

    cb_ptr_pair(const cb_ptr_pair&) noexcept = default;
    cb_ptr_pair& operator=(const cb_ptr_pair&) noexcept = default;

    template <typename U>
    cb_ptr_pair(const cb_ptr_pair<CB, U>& r) noexcept
        : cb(r.cb)
        , ptr(r.ptr)
    {}
    template <typename U>
    cb_ptr_pair& operator=(const cb_ptr_pair<CB, U>& r) noexcept {
        cb = r.cb;
        ptr = r.ptr;
        return *this;
    }

    cb_ptr_pair(cb_ptr_pair&& r) noexcept : cb(r.cb), ptr(r.ptr) {
        r.reset();
    }
    cb_ptr_pair& operator=(cb_ptr_pair&& r) noexcept {
        cb = r.cb;
        ptr = r.ptr;
        r.reset();
        return *this;
    }

    template <typename U>
    cb_ptr_pair(cb_ptr_pair<CB, U>&& r) noexcept : cb(r.cb), ptr(r.ptr) {
        r.reset();
    }
    template <typename U>
    cb_ptr_pair& operator=(cb_ptr_pair<CB, U>&& r) noexcept {
        cb = r.cb;
        ptr = r.ptr;
        r.reset();
        return *this;
    }

    void reset() {
        cb = nullptr;
        ptr = nullptr;
    }
    void swap(cb_ptr_pair& r) {
        std::swap(cb, r.cb);
        std::swap(ptr, r.ptr);
    }

    CB* cb;
    T* ptr;
};

} // namespace xmem
