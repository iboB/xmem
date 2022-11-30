// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "default_delete.hpp"

#include <cstddef> // nullptr_t
#include <utility> // std::forward

namespace xmem {

namespace impl {
template <typename Ptr>
class uptr_common {
protected:
    Ptr m_ptr = Ptr();

    void swap(uptr_common& other) noexcept {
        auto p = m_ptr;
        m_ptr = other.m_ptr;
        other.m_ptr = p;
    }
public:
    uptr_common() noexcept = default;
    uptr_common(Ptr ptr) noexcept : m_ptr(ptr) {}

    uptr_common(const uptr_common&) = delete;
    uptr_common& operator=(const uptr_common&) = delete;

    using pointer = Ptr;

    pointer release() noexcept {
        auto old = m_ptr;
        m_ptr = Ptr();
        return old;
    }

    explicit operator bool() const noexcept { return !!m_ptr; }

    pointer get() const noexcept { return m_ptr; }
    decltype(auto) operator*() const noexcept { return *m_ptr; }
    pointer operator->() const noexcept { return m_ptr; }
};
}

// Unique pointer class to guarantee that the destructor of the managed object
// is called before the pointer itself is ivalidated.
// This allows complex self-referencing from the managed object.
// Not all standard library implementation have this guarantee :(
// More here: https://ibob.bg/blog/2019/11/07/dont-use-unique_ptr-for-pimpl/
template <typename T, typename D = default_delete<T>>
class unique_ptr : private impl::uptr_common<T*>, private D /* inherit from deleter to make use of EBO */ {
    using common = impl::uptr_common<T*>;
public:
    using pointer = typename common::pointer;
    using element_type = T;
    using deleter_type = D;

    unique_ptr() noexcept = default;
    explicit unique_ptr(pointer p) noexcept : common(p) {}
    ~unique_ptr() { D::operator()(this->m_ptr); }

    unique_ptr(std::nullptr_t) noexcept {}
    unique_ptr& operator=(std::nullptr_t) noexcept {
        reset();
        return *this;
    }

    unique_ptr(unique_ptr&& other) noexcept : common(other.release()), D(std::move(other.get_deleter())) {}
    unique_ptr& operator=(unique_ptr&& other) noexcept {
        reset(other.release());
        get_deleter() = std::move(other.get_deleter());
        return *this;
    }

    template <typename D2>
    unique_ptr(pointer p, D2&& d) noexcept : common(p), D(std::forward<D2>(d)) {}

    template <typename U, typename D2>
    unique_ptr(unique_ptr<U, D2>&& other) noexcept : common(other.release()), D(std::move(other.get_deleter())) {}

    void reset(pointer p = pointer()) noexcept {
        auto old = this->m_ptr;
        this->m_ptr = p;
        D::operator()(old);
    }

    void swap(unique_ptr& other) { common::swap(other); }
    using common::release;
    using common::operator bool;
    using common::get;
    using common::operator->;
    using common::operator*;

    D& get_deleter() noexcept { return *this; }
    const D& get_deleter() const noexcept { return *this; }
};

template <typename T, typename... Args>
unique_ptr<T> make_unique(Args&&... args) {
    return unique_ptr<T>{new T(std::forward<Args>(args)...)};
}

template <typename T>
auto make_unique_ptr(T&& t) -> unique_ptr<typename std::remove_reference<T>::type> {
    using RRT = typename std::remove_reference<T>::type;
    return unique_ptr<RRT>(new RRT(std::forward<T>(t)));
}

template <typename T>
unique_ptr<T> make_unique_for_overwrite() {
    return unique_ptr<T>(new T);
}

template <typename T, typename D>
class unique_ptr<T, D&> : public impl::uptr_common<T*> {

};

// compare
template <typename T1, typename D1, typename T2, typename D2>
bool operator==(const xmem::unique_ptr<T1, D1>& u1, const xmem::unique_ptr<T2, D2>& u2) { return u1.get() == u2.get(); }
template <typename T1, typename D1, typename T2, typename D2>
bool operator!=(const xmem::unique_ptr<T1, D1>& u1, const xmem::unique_ptr<T2, D2>& u2) { return u1.get() != u2.get(); }
template <typename T1, typename D1, typename T2, typename D2>
bool operator<(const xmem::unique_ptr<T1, D1>& u1, const xmem::unique_ptr<T2, D2>& u2) { return u1.get() < u2.get(); }
template <typename T1, typename D1, typename T2, typename D2>
bool operator<=(const xmem::unique_ptr<T1, D1>& u1, const xmem::unique_ptr<T2, D2>& u2) { return u1.get() <= u2.get(); }
template <typename T1, typename D1, typename T2, typename D2>
bool operator>(const xmem::unique_ptr<T1, D1>& u1, const xmem::unique_ptr<T2, D2>& u2) { return u1.get() > u2.get(); }
template <typename T1, typename D1, typename T2, typename D2>
bool operator>=(const xmem::unique_ptr<T1, D1>& u1, const xmem::unique_ptr<T2, D2>& u2) { return u1.get() >= u2.get(); }

}