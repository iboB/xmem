// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "basic_shared_ptr.hpp"

namespace xmem {

template <typename T, typename CBT>
class basic_weak_ptr {
public:
    using element_type = std::remove_extent_t<T>;
    using control_block_type = typename CBT::base_type;

    basic_weak_ptr() noexcept : m_cb(nullptr), m_ptr(nullptr) {}

    template <typename U>
    basic_weak_ptr(const basic_weak_ptr<U, CBT>& other) noexcept
        : m_cb(other.m_cb)
        , m_ptr(other.m_ptr)
    {
        if (m_cb) m_cb->inc_weak_ref();
    }
    template <typename U>
    basic_weak_ptr& operator=(const basic_weak_ptr<U, CBT>& other) noexcept {
        if (m_cb) m_cb->dec_weak_ref();
        m_cb = other.m_cb;
        m_ptr = other.m_ptr;
        if (m_cb) m_cb->inc_weak_ref();
        return *this;
    }

    template <typename U>
    basic_weak_ptr(basic_weak_ptr<U, CBT>&& other) noexcept
        : m_cb(other.m_cb)
        , m_ptr(other.m_ptr)
    {
        other.m_cb = nullptr;
        other.m_ptr = nullptr;
    }
    template <typename U>
    basic_weak_ptr& operator=(basic_weak_ptr<U, CBT>&& other) noexcept {
        if (m_cb) m_cb->dec_weak_ref();
        m_cb = other.m_cb;
        m_ptr = other.m_ptr;
        other.m_cb = nullptr;
        other.m_ptr = nullptr;
        return *this;
    }

    template <typename U>
    basic_weak_ptr(const basic_shared_ptr<U, CBT>& sptr) noexcept
        : m_cb(impl::owner_cast<control_block_type>(sptr.owner()))
        , m_ptr(sptr.get())
    {
        if (m_cb) m_cb->inc_weak_ref();
    }
    template <typename U>
    basic_weak_ptr& operator=(const basic_shared_ptr<U, CBT>& sptr) noexcept {
        if (m_cb) m_cb->dec_weak_ref();
        m_cb = impl::owner_cast<control_block_type>(sptr.owner());
        m_ptr = sptr.get();
        if (m_cb) m_cb->inc_weak_ref();
        return *this;
    }

    ~basic_weak_ptr() {
        if (m_cb) m_cb->dec_weak_ref();
    }

    void reset() noexcept {
        if (m_cb) m_cb->dec_weak_ref();
        m_cb = nullptr;
        m_ptr = nullptr;
    }

    void swap(basic_weak_ptr& other) noexcept {
        std::swap(m_cb, other.m_cb);
        std::swap(m_ptr, other.m_ptr);
    }

    [[nodiscard]] long use_count() const noexcept {
        if (!m_cb) return 0;
        return m_cb->strong_ref_count();
    }

    [[nodiscard]] bool expired() const noexcept {
        return use_count() == 0;
    }

    explicit operator bool() const noexcept { return !!m_cb; }

    [[nodiscard]] const void* owner() const noexcept { return m_cb; }

    template <typename U, typename UCBT>
    [[nodiscard]] bool owner_before(const basic_weak_ptr<U, UCBT>& other) const noexcept {
        return m_cb < other.owner();
    }

    template <typename U, typename UCBT>
    [[nodiscard]] bool owner_before(const basic_shared_ptr<U, UCBT>& other) const noexcept {
        return m_cb < other.owner();
    }

    [[nodiscard]] basic_shared_ptr<T, CBT> lock() const noexcept {
        if (!m_cb) return {};

        basic_shared_ptr<T, CBT> ret;
        if (!m_cb->inc_strong_ref_nz()) return {};

        ret.m_ptr = m_ptr;
        ret.m_cb = m_cb;
        return ret;
    }

private:
    control_block_type* m_cb;
    element_type* m_ptr;
};

}