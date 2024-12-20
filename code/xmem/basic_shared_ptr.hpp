// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "cb_ptr_pair.hpp"
#include "unique_ptr.hpp"

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
    basic_shared_ptr(std::nullptr_t) noexcept : basic_shared_ptr() {};

    basic_shared_ptr(const basic_shared_ptr& r) noexcept {
        init_from_copy(r.m);
    }
    basic_shared_ptr& operator=(const basic_shared_ptr& r) noexcept {
        if (&r == this) return *this; // self usurp
        if (m.cb) m.cb->dec_strong_ref(this);
        init_from_copy(r.m);
        return *this;
    }

    template <typename U>
    basic_shared_ptr(const basic_shared_ptr<CBF, U>& r) noexcept {
        init_from_copy(r.m);
    }
    template <typename U>
    basic_shared_ptr& operator=(const basic_shared_ptr<CBF, U>& r) noexcept {
        if (m.cb) m.cb->dec_strong_ref(this);
        init_from_copy(r.m);
        return *this;
    }

    basic_shared_ptr(basic_shared_ptr&& r) noexcept {
        init_from_move(r);
    }
    basic_shared_ptr& operator=(basic_shared_ptr&& r) noexcept {
        if (&r == this) return *this; // self usurp
        if (m.cb) m.cb->dec_strong_ref(this);
        init_from_move(r);
        return *this;
    }

    template <typename U>
    basic_shared_ptr(basic_shared_ptr<CBF, U>&& r) noexcept {
        init_from_move(r);
    }
    template <typename U>
    basic_shared_ptr& operator=(basic_shared_ptr<CBF, U>&& r) noexcept {
        if (m.cb) m.cb->dec_strong_ref(this);
        init_from_move(r);
        return *this;
    }

    template <typename U, typename D>
    basic_shared_ptr(unique_ptr<U, D>&& uptr) {
        init_new(CBF::make_uptr_cb(uptr));
    }

    template <typename U, typename D>
    basic_shared_ptr& operator=(unique_ptr<U, D>&& uptr) {
        if (m.cb) m.cb->dec_strong_ref(this);
        init_new(CBF::make_uptr_cb(uptr));
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

    explicit basic_shared_ptr(cb_ptr_pair_type&& cbptr) noexcept {
        init_new(std::move(cbptr));
    }

    ~basic_shared_ptr() {
        if (m.cb) m.cb->dec_strong_ref(this);
    }

    void reset(std::nullptr_t = nullptr) noexcept {
        if (m.cb) m.cb->dec_strong_ref(this);
        m.reset();
    }

    template <typename U>
    void reset(U* u) {
        operator=(unique_ptr<U>(u));
    }

    template <typename U, typename D>
    void reset(U* u, D d) {
        operator=(unique_ptr<U>(u, std::move(d)));
    }

    template <typename U, typename D, typename A>
    void reset(U* u, D d, A a);

    void swap(basic_shared_ptr& r) noexcept {
        // a self usurp check wouldn't be needed here in a conventional implementation,
        // but we want to make sane transfer_strong calls
        if (m.cb != r.m.cb) {
            if (m.cb) m.cb->transfer_strong(&r, this);
            m.swap(r.m);
            if (m.cb) m.cb->transfer_strong(this, &r);
        }
        else {
            std::swap(m.ptr, r.m.ptr);
        }
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
    [[nodiscard]] const control_block_type* t_owner() const noexcept { return m.cb; }

    template <typename UCBF, typename U>
    [[nodiscard]] bool owner_before(const basic_shared_ptr<UCBF, U>& r) const noexcept {
        return m.cb < r.m.cb;
    }

private:
    // not new! rc taken care of from the outside
    explicit basic_shared_ptr(const cb_ptr_pair_type& cbptr) noexcept : m(cbptr) {}

    template <typename U>
    void init_new(cb_ptr_pair<control_block_type, U>&& r) noexcept {
        m = r;
        if (m.cb) m.cb->init_strong(this);
    }

    template <typename U>
    void init_from_copy(const cb_ptr_pair<control_block_type, U>& r) noexcept {
        m = r;
        if (m.cb) m.cb->inc_strong_ref(this);
    }

    template <typename U>
    void init_from_move(basic_shared_ptr<CBF, U>& r) noexcept {
        m = std::move(r.m);
        if (m.cb) m.cb->transfer_strong(this, &r);
    }

    cb_ptr_pair_type m;

    template <typename, typename> friend class basic_shared_ptr;
    template <typename, typename> friend class basic_weak_ptr;
    template <typename> friend class basic_enable_shared_from;
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

template <typename CBF, typename T>
[[nodiscard]] bool no_owner(const xmem::basic_shared_ptr<CBF, T>& ptr) { return !ptr.owner(); }

template <typename CBF, typename T1, typename T2>
[[nodiscard]] bool same_owner(const xmem::basic_shared_ptr<CBF, T1>& s1, const xmem::basic_shared_ptr<CBF, T2>& s2) {
    return s1.owner() == s2.owner();
}

template <typename CBF, typename U, typename T>
[[nodiscard]] basic_shared_ptr<CBF, T> make_aliased(const basic_shared_ptr<CBF, U>& owner, T* ptr) {
    if (owner.use_count() == 0) return {};
    return basic_shared_ptr<CBF, T>(owner, ptr);
}

// casts

#define I_XMEM_MAKE_POINTER_CAST(cast) \
    template <typename T, typename CBF, typename U> \
    [[nodiscard]] basic_shared_ptr<CBF, T> cast##_pointer_cast(const basic_shared_ptr<CBF, U>& owner) { \
        return make_aliased(owner, cast##_cast<T*>(owner.get())); \
    }

I_XMEM_MAKE_POINTER_CAST(static)
I_XMEM_MAKE_POINTER_CAST(const)
I_XMEM_MAKE_POINTER_CAST(reinterpret)

// not using the macro for dynamic_pointer_cast because we need to check for null
template <typename T, typename CBF, typename U> \
[[nodiscard]] basic_shared_ptr<CBF, T> dynamic_pointer_cast(const basic_shared_ptr<CBF, U>& owner) {
    auto ptr = dynamic_cast<T*>(owner.get());
    if (!ptr) return {};
    return make_aliased(owner, ptr);
}

} // namespace xmem
