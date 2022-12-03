// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "cb_ptr_pair.hpp"
#include "unique_ptr.hpp"
#include "allocator_rebind.hpp"

namespace xmem {

template <typename CBT, typename T>
class basic_weak_ptr;

template <typename CBT, typename T>
class basic_shared_ptr {
public:
    using element_type = std::remove_extent_t<T>;
    using control_block_type = typename CBT::cb_type;
    using weak_type = basic_weak_ptr<CBT, T>;

    basic_shared_ptr() noexcept : m(nullptr) {}
    basic_shared_ptr(std::nullptr_t) noexcept : basic_shared_ptr() {};

    basic_shared_ptr(const basic_shared_ptr& r) noexcept {
        init_from_copy(r);
    }
    basic_shared_ptr& operator=(const basic_shared_ptr& r) noexcept {
        if (m.cb) m.cb->dec_strong_ref();
        init_from_copy(r);
        return *this;
    }

    template <typename U>
    basic_shared_ptr(const basic_shared_ptr<CBT, U>& r) noexcept {
        init_from_copy(r);
    }
    template <typename U>
    basic_shared_ptr& operator=(const basic_shared_ptr<CBT, U>& r) noexcept {
        if (m.cb) m.cb->dec_strong_ref();
        init_from_copy(r);
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
    basic_shared_ptr(basic_shared_ptr<CBT, U>&& r) noexcept {
        init_from_move(r);
    }
    template <typename U>
    basic_shared_ptr& operator=(basic_shared_ptr<CBT, U>&& r) noexcept {
        if (m.cb) m.cb->dec_strong_ref();
        init_from_move(r);
        return *this;
    }

    template <typename U, typename D>
    basic_shared_ptr(unique_ptr<U, D>&& uptr)
        : m(CBT::make_uptr_cb(uptr))
    {}

    template <typename U, typename D>
    basic_shared_ptr& operator=(unique_ptr<U, D>&& uptr) {
        if (m.cb) m.cb->dec_strong_ref();
        m = CBT::make_uptr_cb(uptr);
        return *this;
    }

    template <typename D>
    basic_shared_ptr(std::nullptr_t, D d) : basic_shared_ptr(unique_ptr<T, D>(nullptr, std::move(d))) {}

    template <typename U>
    explicit basic_shared_ptr(U* p) : basic_shared_ptr(unique_ptr<U>(p)) {}

    template <typename U, typename D>
    basic_shared_ptr(U* p, D d) : basic_shared_ptr(unique_ptr<U, D>(p, std::move(d))) {}

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
    [[nodiscard]] T& operator*() const noexcept { return *m.ptr; }
    T* operator->() const noexcept { return m.ptr; }
    [[nodiscard]] element_type& operator[](size_t i) const noexcept {return m.ptr[i]; }

    [[nodiscard]] long use_count() const noexcept {
        if (!m.cb) return 0;
        return m.cb->strong_ref_count();
    }

    explicit operator bool() const noexcept { return !!m.ptr; }

    [[nodiscard]] const void* owner() const noexcept { return m.cb; }

    template <typename UCBT, typename U>
    [[nodiscard]] bool owner_before(const basic_shared_ptr<UCBT, U>& r) const noexcept {
        return m.cb < r.m.cb;
    }

    //template <typename UCBT, typename U, typename Alloc, typename... Args>
    //static basic_shared_ptr<UCBT, U> make(Alloc a, Args&&... args) {
    //    using rsrc_type = typename CBT::template rsrc_type<T, Alloc>;

    //}

private:
    template <typename U>
    void init_from_copy(const basic_shared_ptr<CBT, U>& r) {
        m = r.m;
        if (m.cb) m.cb->inc_strong_ref();
    }

    template <typename U>
    void init_from_move(basic_shared_ptr<CBT, U>& r) {
        m = std::move(r.m);
    }

    cb_ptr_pair<control_block_type, element_type> m;

    template <typename, typename> friend class basic_shared_ptr;
    template <typename, typename> friend class basic_weak_pt;
};

// compare
template <typename CBT1, typename T1, typename CBT2, typename T2>
[[nodiscard]] bool operator==(const xmem::basic_shared_ptr<CBT1, T1>& s1, const xmem::basic_shared_ptr<CBT2, T2>& s2) { return s1.get() == s2.get(); }
template <typename CBT1, typename T1, typename CBT2, typename T2>
[[nodiscard]] bool operator!=(const xmem::basic_shared_ptr<CBT1, T1>& s1, const xmem::basic_shared_ptr<CBT2, T2>& s2) { return s1.get() != s2.get(); }
template <typename CBT1, typename T1, typename CBT2, typename T2>
[[nodiscard]] bool operator<(const xmem::basic_shared_ptr<CBT1, T1>& s1, const xmem::basic_shared_ptr<CBT2, T2>& s2) { return s1.get() < s2.get(); }
template <typename CBT1, typename T1, typename CBT2, typename T2>
[[nodiscard]] bool operator<=(const xmem::basic_shared_ptr<CBT1, T1>& s1, const xmem::basic_shared_ptr<CBT2, T2>& s2) { return s1.get() <= s2.get(); }
template <typename CBT1, typename T1, typename CBT2, typename T2>
[[nodiscard]] bool operator>(const xmem::basic_shared_ptr<CBT1, T1>& s1, const xmem::basic_shared_ptr<CBT2, T2>& s2) { return s1.get() > s2.get(); }
template <typename CBT1, typename T1, typename CBT2, typename T2>
[[nodiscard]] bool operator>=(const xmem::basic_shared_ptr<CBT1, T1>& s1, const xmem::basic_shared_ptr<CBT2, T2>& s2) { return s1.get() >= s2.get(); }

} // namespace xmem
