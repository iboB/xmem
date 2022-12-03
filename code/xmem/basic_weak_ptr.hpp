// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "basic_shared_ptr.hpp"

namespace xmem {

template <typename CBF, typename T>
class basic_weak_ptr {
public:
    using element_type = std::remove_extent_t<T>;
    using control_block_type = typename CBF::cb_type;

    basic_weak_ptr() noexcept : m(nullptr) {}

    basic_weak_ptr(const basic_weak_ptr& r) noexcept {
        init_from_copy(r);
    }
    basic_weak_ptr& operator=(const basic_weak_ptr& r) noexcept {
        if (m.cb) m.cb->dec_weak_ref();
        init_from_copy(r);
        return *this;
    }

    template <typename U>
    basic_weak_ptr(const basic_weak_ptr<CBF, U>& r) noexcept {
        init_from_copy(r);
    }
    template <typename U>
    basic_weak_ptr& operator=(const basic_weak_ptr<CBF, U>& r) noexcept {
        if (m.cb) m.cb->dec_weak_ref();
        init_from_copy(r);
        return *this;
    }

    basic_weak_ptr(basic_weak_ptr&& r) noexcept {
        init_from_move(r);
    }
    basic_weak_ptr& operator=(basic_weak_ptr&& r) noexcept {
        if (m.cb) m.cb->dec_weak_ref();
        init_from_move(r);
        return *this;
    }

    template <typename U>
    basic_weak_ptr(basic_weak_ptr<CBF, U>&& r) noexcept {
        init_from_move(r);
    }
    template <typename U>
    basic_weak_ptr& operator=(basic_weak_ptr<CBF, U>&& r) noexcept {
        if (m.cb) m.cb->dec_weak_ref();
        init_from_move(r);
        return *this;
    }

    template <typename U>
    basic_weak_ptr(const basic_shared_ptr<CBF, U>& sptr) noexcept
        : m(sptr.m)
    {
        if (m.cb) m.cb->inc_weak_ref();
    }
    template <typename U>
    basic_weak_ptr& operator=(const basic_shared_ptr<CBF, U>& sptr) noexcept {
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

    template <typename UCBF, typename U>
    [[nodiscard]] bool owner_before(const basic_weak_ptr<UCBF, U>& r) const noexcept {
        return m.cb < r.m.cb;
    }

    template <typename UCBF, typename U>
    [[nodiscard]] bool owner_before(const basic_shared_ptr<UCBF, U>& r) const noexcept {
        return m.cb < r.m.cb;
    }

    [[nodiscard]] basic_shared_ptr<CBF, T> lock() const noexcept {
        if (!m.cb) return {};

        basic_shared_ptr<CBF, T> ret;
        if (!m.cb->inc_strong_ref_nz()) return {};

        ret.m = m;
        return ret;
    }

private:
    template <typename U>
    void init_from_copy(const basic_weak_ptr<CBF, U>& r) {
        m = r.m;
        if (m.cb) m.cb->inc_weak_ref();
    }

    template <typename U>
    void init_from_move(basic_weak_ptr<CBF, U>& r) {
        m = std::move(r.m);
    }

    cb_ptr_pair<control_block_type, element_type> m;
};

} // namespace xmem
