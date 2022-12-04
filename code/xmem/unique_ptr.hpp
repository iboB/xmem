// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "default_delete.hpp"

#include <cstddef> // nullptr_t, size_t
#include <utility> // std::forward, std::swap

namespace xmem {

namespace impl {
template <typename Ptr>
class uptr_common {
protected:
    Ptr m_ptr = Ptr();

    void swap(uptr_common& other) noexcept {
        std::swap(m_ptr, other.m_ptr);
    }
public:
    uptr_common() noexcept = default;
    uptr_common(Ptr ptr) noexcept : m_ptr(ptr) {}

    uptr_common(const uptr_common&) = delete;
    uptr_common& operator=(const uptr_common&) = delete;

    using pointer = Ptr;

    [[nodiscard]] pointer release() noexcept {
        auto old = m_ptr;
        m_ptr = Ptr();
        return old;
    }

    explicit operator bool() const noexcept { return !!m_ptr; }

    [[nodiscard]] pointer get() const noexcept { return m_ptr; }
    [[nodiscard]] decltype(auto) operator*() const noexcept { return *m_ptr; }
    pointer operator->() const noexcept { return m_ptr; }

    [[nodiscard]] decltype(auto) operator[](size_t i) const noexcept { return m_ptr[i]; }
};
}

// Unique pointer class to guarantee that the destructor of the managed object
// is called before the pointer itself is ivalidated.
// This allows complex self-referencing from the managed object.
// Not all standard library implementations have this guarantee :(
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
    ~unique_ptr() {
        if (this->m_ptr) {
            D::operator()(this->m_ptr);
        }
    }

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
    template <typename U, typename D2>
    unique_ptr& operator=(unique_ptr<U, D2>&& other) noexcept {
        reset(other.release());
        get_deleter() = std::move(other.get_deleter());
        return *this;
    }

    void reset(pointer p = pointer()) noexcept {
        auto old = this->m_ptr;
        this->m_ptr = p;
        if (old) {
            D::operator()(old);
        }
    }

    void swap(unique_ptr& other) { common::swap(other); }
    using common::release;
    using common::operator bool;
    using common::get;
    using common::operator->;
    using common::operator*;

    [[nodiscard]] D& get_deleter() noexcept { return *this; }
    [[nodiscard]] const D& get_deleter() const noexcept { return *this; }
};

template <typename T, typename... Args>
[[nodiscard]] std::enable_if_t<!std::is_array_v<T>, unique_ptr<T>>
make_unique(Args&&... args) {
    return unique_ptr<T>{new T(std::forward<Args>(args)...)};
}

template <typename T>
[[nodiscard]] auto make_unique_ptr(T&& t) -> unique_ptr<std::remove_reference_t<T>> {
    using RRT = std::remove_reference_t<T>;
    return unique_ptr<RRT>(new RRT(std::forward<T>(t)));
}

template <typename T>
[[nodiscard]] std::enable_if_t<!std::is_array_v<T>, unique_ptr<T>>
make_unique_for_overwrite() {
    return unique_ptr<T>(new T);
}

template <typename T, typename D>
class unique_ptr<T, D&> : public impl::uptr_common<T*> {
    using common = impl::uptr_common<T*>;
    D* m_deleter;
public:
    using pointer = typename common::pointer;
    using element_type = T;
    using deleter_type = D&;

    unique_ptr(pointer p, D& d) noexcept : common(p), m_deleter(&d) {}
    explicit unique_ptr(D& d) noexcept : common(nullptr), m_deleter(&d) {}
    ~unique_ptr() {
        if (this->m_ptr) {
            (*m_deleter)(this->m_ptr);
        }
    }

    unique_ptr& operator=(std::nullptr_t) noexcept {
        reset();
        return *this;
    }

    unique_ptr(unique_ptr&& other) noexcept : common(other.release()), m_deleter(&other.get_deleter()) {}
    unique_ptr& operator=(unique_ptr&& other) noexcept {
        reset(other.release());
        m_deleter = &other.get_deleter();
        return *this;
    }

    template <typename U, typename D2>
    unique_ptr(unique_ptr<U, D2>&& other) noexcept : common(other.release()), m_deleter(&other.get_deleter()) {}
    template <typename U, typename D2>
    unique_ptr& operator=(unique_ptr<U, D2>&& other) noexcept {
        reset(other.release());
        m_deleter = &other.get_deleter();
        return *this;
    }

    void reset(pointer p = pointer()) noexcept {
        auto old = this->m_ptr;
        this->m_ptr = p;
        if (old) {
            (*m_deleter)(old);
        }
    }

    void swap(unique_ptr& other) { common::swap(other); }
    using common::release;
    using common::operator bool;
    using common::get;
    using common::operator->;
    using common::operator*;

    [[nodiscard]] D& get_deleter() noexcept { return *m_deleter; }
    [[nodiscard]] const D& get_deleter() const noexcept { return *m_deleter; }
};

template <typename T, typename B>
class unique_ptr<T, void(*)(B*)> : public impl::uptr_common<T*> {
    static_assert(std::is_convertible_v<B*, T*>, "unique_ptr type needs to be convertible to deleter type");
    using common = impl::uptr_common<T*>;
    void (*m_deleter)(B*);
public:
    using pointer = typename common::pointer;
    using element_type = T;
    using deleter_type = void(*)(B*);

    unique_ptr(pointer p, deleter_type d) noexcept : common(p), m_deleter(d) {}
    unique_ptr(pointer, std::nullptr_t) = delete;
    explicit unique_ptr(deleter_type d) noexcept : common(nullptr), m_deleter(d) {}
    explicit unique_ptr(std::nullptr_t) = delete;
    ~unique_ptr() {
        if (this->m_ptr) {
            m_deleter(this->m_ptr);
        }
    }

    unique_ptr& operator=(std::nullptr_t) noexcept {
        reset();
        return *this;
    }

    unique_ptr(unique_ptr&& other) noexcept : common(other.release()), m_deleter(other.get_deleter()) {}
    unique_ptr& operator=(unique_ptr&& other) noexcept {
        reset(other.release());
        m_deleter = other.get_deleter();
        return *this;
    }

    template <typename U>
    unique_ptr(unique_ptr<U, deleter_type>&& other) noexcept : common(other.release()), m_deleter(other.get_deleter()) {}
    template <typename U, typename D2>
    unique_ptr& operator=(unique_ptr<U, D2>&& other) noexcept {
        reset(other.release());
        m_deleter = other.get_deleter();
        return *this;
    }

    void reset(pointer p = pointer()) noexcept {
        auto old = this->m_ptr;
        this->m_ptr = p;
        if (old) {
            m_deleter(old);
        }
    }

    void swap(unique_ptr& other) { common::swap(other); }
    using common::release;
    using common::operator bool;
    using common::get;
    using common::operator->;
    using common::operator*;

    [[nodiscard]] deleter_type get_deleter() const noexcept { return *m_deleter; }
};

template <typename T, typename D>
class unique_ptr<T[], D> : private impl::uptr_common<T*>, private D /* inherit from deleter to make use of EBO */ {
    using common = impl::uptr_common<T*>;
public:
    using pointer = typename common::pointer;
    using element_type = T;
    using deleter_type = D;

    unique_ptr() noexcept = default;
    explicit unique_ptr(pointer p) noexcept : common(p) {}
    ~unique_ptr() {
        if (this->m_ptr) {
            D::operator()(this->m_ptr);
        }
    }

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

    void reset(pointer p = pointer()) noexcept {
        auto old = this->m_ptr;
        this->m_ptr = p;
        if (old) {
            D::operator()(old);
        }
    }

    void swap(unique_ptr& other) { common::swap(other); }
    using common::release;
    using common::operator bool;
    using common::get;
    using common::operator[];

    [[nodiscard]] D& get_deleter() noexcept { return *this; }
    [[nodiscard]] const D& get_deleter() const noexcept { return *this; }
};

template <typename T, typename... Args>
[[nodiscard]] std::enable_if_t<std::is_array_v<T>, unique_ptr<T>>
make_unique(size_t n) {
    return unique_ptr<T>{new std::remove_extent_t<T>[n]()};
}
template <typename T>
[[nodiscard]] std::enable_if_t<std::is_array_v<T>, unique_ptr<T>>
make_unique_for_overwrite(size_t n) {
    return unique_ptr<T>{new std::remove_extent_t<T>[n]};
}


// compare
template <typename T1, typename D1, typename T2, typename D2>
[[nodiscard]] bool operator==(const unique_ptr<T1, D1>& u1, const unique_ptr<T2, D2>& u2) { return u1.get() == u2.get(); }
template <typename T1, typename D1, typename T2, typename D2>
[[nodiscard]] bool operator!=(const unique_ptr<T1, D1>& u1, const unique_ptr<T2, D2>& u2) { return u1.get() != u2.get(); }
template <typename T1, typename D1, typename T2, typename D2>
[[nodiscard]] bool operator<(const unique_ptr<T1, D1>& u1, const unique_ptr<T2, D2>& u2) { return u1.get() < u2.get(); }
template <typename T1, typename D1, typename T2, typename D2>
[[nodiscard]] bool operator<=(const unique_ptr<T1, D1>& u1, const unique_ptr<T2, D2>& u2) { return u1.get() <= u2.get(); }
template <typename T1, typename D1, typename T2, typename D2>
[[nodiscard]] bool operator>(const unique_ptr<T1, D1>& u1, const unique_ptr<T2, D2>& u2) { return u1.get() > u2.get(); }
template <typename T1, typename D1, typename T2, typename D2>
[[nodiscard]] bool operator>=(const unique_ptr<T1, D1>& u1, const unique_ptr<T2, D2>& u2) { return u1.get() >= u2.get(); }

} // namespace xmem