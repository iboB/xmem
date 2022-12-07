// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "cb_ptr_pair.hpp"
#include "unique_ptr.hpp"
#include "allocator_rebind.hpp"

namespace xmem {

template <typename CBF, typename T>
class basic_weak_ptr;

template <typename CBF>
class basic_enable_shared_from;

template <typename CBF, typename T>
class basic_shared_ptr {
public:
    using element_type = std::remove_extent_t<T>;
    using control_block_type = typename CBF::cb_type;
    using weak_type = basic_weak_ptr<CBF, T>;
    using cb_ptr_pair_type = cb_ptr_pair<control_block_type, element_type>;

    basic_shared_ptr() noexcept : m(nullptr) {}
    explicit basic_shared_ptr(const cb_ptr_pair_type& cbptr) noexcept : m(cbptr) {}
    basic_shared_ptr(std::nullptr_t) noexcept : basic_shared_ptr() {};

    basic_shared_ptr(const basic_shared_ptr& r) noexcept {
        init_from_copy(r.m);
    }
    basic_shared_ptr& operator=(const basic_shared_ptr& r) noexcept {
        if (m.cb) m.cb->dec_strong_ref();
        init_from_copy(r.m);
        return *this;
    }

    template <typename U>
    basic_shared_ptr(const basic_shared_ptr<CBF, U>& r) noexcept {
        init_from_copy(r.m);
    }
    template <typename U>
    basic_shared_ptr& operator=(const basic_shared_ptr<CBF, U>& r) noexcept {
        if (m.cb) m.cb->dec_strong_ref();
        init_from_copy(r.m);
        return *this;
    }

    basic_shared_ptr(basic_shared_ptr&& r) noexcept {
        init_from_move(r);
    }
    basic_shared_ptr& operator=(basic_shared_ptr&& r) noexcept {
        if (m.cb) m.cb->dec_strong_ref();
        init_from_move(r);
        return *this;
    }

    template <typename U>
    basic_shared_ptr(basic_shared_ptr<CBF, U>&& r) noexcept {
        init_from_move(r);
    }
    template <typename U>
    basic_shared_ptr& operator=(basic_shared_ptr<CBF, U>&& r) noexcept {
        if (m.cb) m.cb->dec_strong_ref();
        init_from_move(r);
        return *this;
    }

    template <typename U, typename D>
    basic_shared_ptr(unique_ptr<U, D>&& uptr)
        : m(CBF::make_uptr_cb(uptr))
    {}

    template <typename U, typename D>
    basic_shared_ptr& operator=(unique_ptr<U, D>&& uptr) {
        if (m.cb) m.cb->dec_strong_ref();
        m = CBF::make_uptr_cb(uptr);
        return *this;
    }

    template <typename D>
    basic_shared_ptr(std::nullptr_t, D d) : basic_shared_ptr(unique_ptr<T, D>(nullptr, std::move(d))) {}

    template <typename U>
    explicit basic_shared_ptr(U* p) : basic_shared_ptr(unique_ptr<U>(p)) {}

    template <typename U, typename D>
    basic_shared_ptr(U* p, D d) : basic_shared_ptr(unique_ptr<U, D>(p, std::move(d))) {}

    template <typename U>
    basic_shared_ptr(const basic_shared_ptr<CBF, U>& r, T* aptr) noexcept {
        init_from_copy(cb_ptr_pair_type(r.m.cb, aptr));
    }

    ~basic_shared_ptr() {
        if (m.cb) m.cb->dec_strong_ref();
    }

    void reset(std::nullptr_t = nullptr) noexcept {
        if (m.cb) {
            m.cb->dec_strong_ref();
            m.reset();
        }
    }

    template <typename U>
    void reset(U* u);

    template <typename U, typename D>
    void reset(U* u, D d);

    template <typename U, typename D, typename A>
    void reset(U* u, D d, A a);

    void swap(basic_shared_ptr& r) noexcept {
        m.swap(r.m);
    }

    [[nodiscard]] element_type* get() const noexcept { return m.ptr; }

    template <typename TT = T, typename = std::enable_if_t<!std::is_void_v<TT>>>
    [[nodiscard]] TT& operator*() const noexcept { return *m.ptr; }

    T* operator->() const noexcept { return m.ptr; }

    template <typename TT = T, typename Elem = element_type, typename = std::enable_if_t<!std::is_void_v<TT>> >
    [[nodiscard]] Elem& operator[](size_t i) const noexcept { return m.ptr[i]; }

    [[nodiscard]] long use_count() const noexcept {
        if (!m.cb) return 0;
        return m.cb->strong_ref_count();
    }

    explicit operator bool() const noexcept { return !!m.ptr; }

    [[nodiscard]] const void* owner() const noexcept { return m.cb; }

    template <typename UCBF, typename U>
    [[nodiscard]] bool owner_before(const basic_shared_ptr<UCBF, U>& r) const noexcept {
        return m.cb < r.m.cb;
    }

private:
    template <typename U>
    void init_from_copy(const cb_ptr_pair<control_block_type, U>& r) noexcept {
        m = r;
        if (m.cb) m.cb->inc_strong_ref();
    }

    template <typename U>
    void init_from_move(basic_shared_ptr<CBF, U>& r) noexcept {
        m = std::move(r.m);
    }

    cb_ptr_pair_type m;

    template <typename, typename> friend class basic_shared_ptr;
    template <typename, typename> friend class basic_weak_ptr;
    template <typename> class basic_enable_shared_from;
};

// compare
template <typename CBF1, typename T1, typename CBF2, typename T2>
[[nodiscard]] bool operator==(const xmem::basic_shared_ptr<CBF1, T1>& s1, const xmem::basic_shared_ptr<CBF2, T2>& s2) { return s1.get() == s2.get(); }
template <typename CBF1, typename T1, typename CBF2, typename T2>
[[nodiscard]] bool operator!=(const xmem::basic_shared_ptr<CBF1, T1>& s1, const xmem::basic_shared_ptr<CBF2, T2>& s2) { return s1.get() != s2.get(); }
template <typename CBF1, typename T1, typename CBF2, typename T2>
[[nodiscard]] bool operator<(const xmem::basic_shared_ptr<CBF1, T1>& s1, const xmem::basic_shared_ptr<CBF2, T2>& s2) { return s1.get() < s2.get(); }
template <typename CBF1, typename T1, typename CBF2, typename T2>
[[nodiscard]] bool operator<=(const xmem::basic_shared_ptr<CBF1, T1>& s1, const xmem::basic_shared_ptr<CBF2, T2>& s2) { return s1.get() <= s2.get(); }
template <typename CBF1, typename T1, typename CBF2, typename T2>
[[nodiscard]] bool operator>(const xmem::basic_shared_ptr<CBF1, T1>& s1, const xmem::basic_shared_ptr<CBF2, T2>& s2) { return s1.get() > s2.get(); }
template <typename CBF1, typename T1, typename CBF2, typename T2>
[[nodiscard]] bool operator>=(const xmem::basic_shared_ptr<CBF1, T1>& s1, const xmem::basic_shared_ptr<CBF2, T2>& s2) { return s1.get() >= s2.get(); }

} // namespace xmem
