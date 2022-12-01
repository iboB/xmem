// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "unique_ptr.hpp"

namespace xmem {

namespace impl {
template <typename CB>
CB* owner_cast(const void* owner) {
    return reinterpret_cast<CB*>(const_cast<void*>(owner));
}
}

template <typename T, typename CBT>
class basic_weak_ptr;

template <typename T, typename CBT>
class basic_shared_ptr {
public:
    using element_type = std::remove_extent_t<T>;
    using control_block_type = typename CBT::base_type;
    using weak_type = basic_weak_ptr<T, CBT>;

    basic_shared_ptr() noexcept : m_cb(nullptr), m_ptr(nullptr) {}
    basic_shared_ptr(nullptr_t) noexcept : basic_shared_ptr() {};

    template <typename U>
    basic_shared_ptr(const basic_shared_ptr<U, CBT>& other) noexcept
        : m_cb(other.m_cb)
        , m_ptr(other.m_ptr)
    {
        if (m_cb) m_cb->inc_strong_ref();
    }
    template <typename U>
    basic_shared_ptr& operator=(const basic_shared_ptr<U, CBT>& other) noexcept {
        if (m_cb) m_cb->dec_strong_ref();
        m_cb = other.m_cb;
        m_ptr = other.m_ptr;
        if (m_cb) m_cb->inc_strong_ref();
        return *this;
    }

    template <typename U>
    basic_shared_ptr(basic_shared_ptr<U, CBT>&& other) noexcept
        : m_cb(other.m_cb)
        , m_ptr(other.m_ptr)
    {
        other.m_cb = nullptr;
        other.m_ptr = nullptr;
    }

    template <typename U>
    basic_shared_ptr& operator=(basic_shared_ptr<U, CBT>&& other) noexcept {
        if (m_cb) m_cb->dec_strong_ref();
        m_cb = other.m_cb;
        m_ptr = other.m_ptr;
        other.m_cb = nullptr;
        other.m_ptr = nullptr;
        return *this;
    }

    template <typename U, typename D>
    basic_shared_ptr(unique_ptr<U, D>&& uptr) {
        m_ptr = uptr.get();
        m_cb = new typename CBT::template uptr_type<U, D>(uptr);
    }

    template <typename U, typename D>
    basic_shared_ptr& operator=(unique_ptr<U, D>&& uptr) {
        if (m_cb) m_cb->dec_strong_ref();
        m_ptr = uptr.get();
        m_cb = new typename CBT::template uptr_type<U, D>(uptr);
        return *this;
    }

    template <typename D>
    basic_shared_ptr(nullptr_t, D d) : basic_shared_ptr(unique_ptr<T, D>(nullptr, std::move(d))) {}

    template <typename U>
    explicit basic_shared_ptr(U* p) : basic_shared_ptr(unique_ptr<U>(p)) {}

    template <typename U, typename D>
    basic_shared_ptr(U* p, D d) : basic_shared_ptr(unique_ptr<U, D>(p, std::move(d))) {}

    ~basic_shared_ptr() {
        if (m_cb) m_cb->dec_strong_ref();
    }

    void reset(nullptr_t = nullptr) noexcept {
        if (m_cb) {
            m_cb->dec_strong_ref();
            m_cb = nullptr;
            m_ptr = nullptr;
        }
    }

    template <typename U>
    void reset(U* u);

    template <typename U, typename D>
    void reset(U* u, D d);

    template <typename U, typename D, typename A>
    void reset(U* u, D d, A a);

    void swap(basic_shared_ptr& other) noexcept {
        std::swap(m_cb, other.m_cb);
        std::swap(m_ptr, other.m_ptr);
    }

    [[nodiscard]] element_type* get() const noexcept { return m_ptr; }
    [[nodiscard]] T& operator*() const noexcept { return *m_ptr; }
    T* operator->() const noexcept { return m_ptr; }
    [[nodiscard]] element_type& operator[](size_t i) const noexcept {return m_ptr[i]; }

    [[nodiscard]] long use_count() const noexcept {
        if (!m_cb) return 0;
        return m_cb->strong_ref_count();
    }

    explicit operator bool() const noexcept { return !!m_ptr; }

    [[nodiscard]] const void* owner() const noexcept { return m_cb; }

    template <typename U, typename UCBT>
    [[nodiscard]] bool owner_before(const basic_shared_ptr<U, UCBT>& other) const noexcept {
        return m_cb < other.owner();
    }

private:
    control_block_type* m_cb;
    element_type* m_ptr;
};


}
