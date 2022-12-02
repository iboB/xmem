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

    basic_weak_ptr() noexcept : m(nullptr) {}

    template <typename U>
    basic_weak_ptr(const basic_weak_ptr<U, CBT>& r) noexcept
        : m(r.m)
    {
        if (m.cb) m.cb->inc_weak_ref();
    }
    template <typename U>
    basic_weak_ptr& operator=(const basic_weak_ptr<U, CBT>& r) noexcept {
        if (m.cb) m.cb->dec_weak_ref();
        m = r.m;
        if (m.cb) m.cb->inc_weak_ref();
        return *this;
    }

    template <typename U>
    basic_weak_ptr(basic_weak_ptr<U, CBT>&& r) noexcept
        : m(std::move(r.m))
    {}
    template <typename U>
    basic_weak_ptr& operator=(basic_weak_ptr<U, CBT>&& r) noexcept {
        if (m.cb) m.cb->dec_weak_ref();
        m = std::move(r.m);
        return *this;
    }

    template <typename U>
    basic_weak_ptr(const basic_shared_ptr<U, CBT>& sptr) noexcept
        : m(sptr.m)
    {
        if (m.cb) m.cb->inc_weak_ref();
    }
    template <typename U>
    basic_weak_ptr& operator=(const basic_shared_ptr<U, CBT>& sptr) noexcept {
        if (m.cb) m.cb->dec_weak_ref();
        m = sptr.m;
        if (m.cb) m.cb->inc_weak_ref();
        return *this;
    }

    ~basic_weak_ptr() {
        if (m.cb) m.cb->dec_weak_ref();
    }

    void reset() noexcept {
        if (m.cb) {
            m.cb->dec_weak_ref();
            m.reset();
        }
    }

    void swap(basic_weak_ptr& r) noexcept {
        m.swap(r.m);
    }

    [[nodiscard]] long use_count() const noexcept {
        if (!m.cb) return 0;
        return m.cb->strong_ref_count();
    }

    [[nodiscard]] bool expired() const noexcept {
        return use_count() == 0;
    }

    explicit operator bool() const noexcept { return !!m.cb; }

    [[nodiscard]] const void* owner() const noexcept { return m.cb; }

    template <typename U, typename UCBT>
    [[nodiscard]] bool owner_before(const basic_weak_ptr<U, UCBT>& r) const noexcept {
        return m.cb < r.m.cb;
    }

    template <typename U, typename UCBT>
    [[nodiscard]] bool owner_before(const basic_shared_ptr<U, UCBT>& r) const noexcept {
        return m.cb < r.m.cb;
    }

    [[nodiscard]] basic_shared_ptr<T, CBT> lock() const noexcept {
        if (!m.cb) return {};

        basic_shared_ptr<T, CBT> ret;
        if (!m.cb->inc_strong_ref_nz()) return {};

        ret.m = m;
        return ret;
    }

private:
    impl::cb_ptr_pair<control_block_type, element_type> m;
};

} // namespace xmem
