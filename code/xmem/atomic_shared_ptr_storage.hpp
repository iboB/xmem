// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "bits/spinlock.hpp"
#include "shared_ptr.hpp"

namespace xmem {

template <typename T>
class alignas(impl::cache_line_size) atomic_shared_ptr_storage {
    shared_ptr<T> m_ptr;
    mutable impl::spinlock m_spinlock;
public:
    using shared_pointer_type = shared_ptr<T>;

    atomic_shared_ptr_storage() noexcept = default;
    atomic_shared_ptr_storage(shared_pointer_type ptr) noexcept : m_ptr(std::move(ptr)) {}

    atomic_shared_ptr_storage(const atomic_shared_ptr_storage&) = delete;
    atomic_shared_ptr_storage& operator=(const atomic_shared_ptr_storage&) = delete;

    shared_pointer_type load() const noexcept {
        impl::spinlock::lock_guard _l(m_spinlock);
        return m_ptr;
    }
    void store(shared_pointer_type ptr) noexcept {
        impl::spinlock::lock_guard _l(m_spinlock);
        m_ptr.swap(ptr);
    }

    shared_pointer_type exchange(shared_pointer_type ptr) noexcept {
        {
            impl::spinlock::lock_guard _l(m_spinlock);
            m_ptr.swap(ptr);
        }
        return std::move(ptr);
    }

    bool compare_exchange(shared_pointer_type& expect, shared_pointer_type ptr) noexcept {
        impl::spinlock::lock_guard _l(m_spinlock);
        if (m_ptr == expect) {
            m_ptr.swap(ptr);
            return true;
        }
        else {
            expect = m_ptr;
            return false;
        }
    }
};

}
