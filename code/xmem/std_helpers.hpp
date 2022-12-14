// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "bits/spinlock.hpp"

#include <memory>
#include <atomic>

namespace xmem::xstd {

template <typename U, typename T>
std::shared_ptr<T> make_aliased(const std::shared_ptr<U>& owner, T* ptr) {
    if (owner.use_count() == 0) return {};
    return std::shared_ptr<T>(owner, ptr);
}

namespace impl {
#if __cplusplus >= 202000L && defined(__cpp_lib_atomic_shared_ptr)
// do what the stdlib implementers chose as best in case it's available
template <typename T>
using asps_holder = std::atomic<std::shared_ptr<T>>;
#else
template <typename T>
class asps_holder {
    using sptr = std::shared_ptr<T>;
    sptr m_ptr;
    mutable xmem::impl::spinlock m_spinlock;
public:
    asps_holder() noexcept = default;
    asps_holder(std::shared_ptr<T> ptr) noexcept : m_ptr(std::move(ptr)) {}

    sptr load() const noexcept {
        xmem::impl::spinlock::lock_guard _l(m_spinlock);
        return m_ptr;
    }

    void store(sptr ptr) noexcept {
        xmem::impl::spinlock::lock_guard _l(m_spinlock);
        m_ptr.swap(ptr);
    }

    sptr exchange(sptr ptr) noexcept {
        {
            xmem::impl::spinlock::lock_guard _l(m_spinlock);
            m_ptr.swap(ptr);
        }
        return std::move(ptr);
    }

    // have _strong to match atomic<shared_ptr>
    bool compare_exchange_strong(sptr& expect, sptr ptr) noexcept {
        xmem::impl::spinlock::lock_guard _l(m_spinlock);
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
#endif
}

template <typename T>
class atomic_shared_ptr_storage {
    impl::asps_holder<T> m_holder;
public:
    using shared_pointer_type = std::shared_ptr<T>;

    atomic_shared_ptr_storage() noexcept = default;
    atomic_shared_ptr_storage(shared_pointer_type ptr) noexcept : m_holder(std::move(ptr)) {}

    atomic_shared_ptr_storage(const atomic_shared_ptr_storage&) = delete;
    atomic_shared_ptr_storage& operator=(const atomic_shared_ptr_storage&) = delete;

    shared_pointer_type load() const noexcept { return m_holder.load(); }
    void store(shared_pointer_type ptr) noexcept { m_holder.store(std::move(ptr)); }

    shared_pointer_type exchange(shared_pointer_type ptr) noexcept { return m_holder.exchange(std::move(ptr)); }

    bool compare_exchange(shared_pointer_type& expect, shared_pointer_type ptr) noexcept {
        return m_holder.compare_exchange_strong(expect, std::move(ptr));
    }
};

}
