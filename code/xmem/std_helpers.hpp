// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "bits/spinlock.hpp"

#include <memory>
#include <atomic>

namespace xstd {

template <typename T>
auto make_shared_ptr(T&& t) -> std::shared_ptr<typename std::remove_reference<T>::type>
{
    return std::make_shared<typename std::remove_reference<T>::type>(std::forward<T>(t));
}

template <typename T>
auto make_unique_ptr(T&& t) -> std::unique_ptr<typename std::remove_reference<T>::type>
{
    using RRT = typename std::remove_reference<T>::type;
    return  std::unique_ptr<RRT>(new RRT(std::forward<T>(t)));
}

template <typename U, typename T>
std::shared_ptr<T> make_aliased(const std::shared_ptr<U>& owner, T* ptr) {
    if (owner.use_count() == 0) return {};
    return std::shared_ptr<T>(owner, ptr);
}

template <typename T, typename U>
bool same_owner(const std::shared_ptr<T>& a, const std::shared_ptr<U>& b) {
    return !a.owner_before(b) && !b.owner_before(a);
}
template <typename T, typename U>
bool same_owner(const std::weak_ptr<T>& a, const std::weak_ptr<U>& b) {
    return !a.owner_before(b) && !b.owner_before(a);
}
template <typename T, typename U>
bool same_owner(const std::weak_ptr<T>& a, const std::shared_ptr<U>& b) {
    return !a.owner_before(b) && !b.owner_before(a);
}
template <typename T, typename U>
bool same_owner(const std::shared_ptr<T>& a, const std::weak_ptr<U>& b) {
    return !a.owner_before(b) && !b.owner_before(a);
}

template <typename T>
bool no_owner(const std::shared_ptr<T>& ptr) {
    std::shared_ptr<T> e;
    return same_owner(ptr, e);
}
template <typename T>
bool no_owner(const std::weak_ptr<T>& ptr) {
    std::weak_ptr<T> e;
    return same_owner(ptr, e);
}

class enable_shared_from : public std::enable_shared_from_this<enable_shared_from>
{
    using esd = std::enable_shared_from_this<enable_shared_from>;
protected:
    std::shared_ptr<void> shared_from_this() { return esd::shared_from_this(); }
    std::shared_ptr<const void> shared_from_this() const { return esd::shared_from_this(); }

    std::weak_ptr<void> weak_from_this()
    {
        return esd::weak_from_this();
    }

    std::weak_ptr<const void> weak_from_this() const
    {
        return esd::weak_from_this();
    }

    template <typename T>
    std::shared_ptr<T> shared_from(T* ptr) const {
        return std::shared_ptr<T>(shared_from_this(), ptr);
    }

    template <typename T>
    std::weak_ptr<T> weak_from(T* ptr) const {
        // c++ doesn't have an aliasing weak_ptr
        return shared_from(ptr);
    }
};


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
class alignas(xmem::impl::cache_line_size) atomic_shared_ptr_storage {
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
