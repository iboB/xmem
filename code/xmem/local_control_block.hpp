// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "cb_ptr_pair.hpp"
#include "unique_ptr.hpp"
#include "allocator_rebind.hpp"
#include "allocator.hpp"

#include <new>

namespace xmem {

class local_control_block_base {
    uint32_t m_strong_refs = 1;
    uint32_t m_weak_refs = 1;
public:
    virtual ~local_control_block_base() noexcept = default;

    virtual void destroy_resource() noexcept = 0;
    virtual void destroy_self() noexcept = 0;

    void inc_strong_ref() noexcept { ++m_strong_refs; }
    void dec_strong_ref() noexcept {
        if (--m_strong_refs == 0) {
            destroy_resource();
            dec_weak_ref();
        }
    }
    uint32_t int_strong_ref_nz() noexcept {
        if (!m_strong_refs) return 0;
        return ++m_strong_refs;
    }

    void inc_weak_ref() noexcept { ++m_weak_refs; }
    void dec_weak_ref() noexcept {
        if (--m_weak_refs == 0) {
            destroy_self();
        }
    }

    long strong_ref_count() const noexcept {
        return long(m_strong_refs);
    }
};

template <typename T, typename D>
class local_control_block_uptr final : public local_control_block_base {
    using uptr_t = unique_ptr<T, D>;
    uptr_t m_uptr;
public:
    local_control_block_uptr(uptr_t& uptr) noexcept : m_uptr(std::move(uptr)) {}

    [[nodiscard]] T* ptr() {
        return m_uptr.get();
    }

    virtual void destroy_resource() noexcept override { m_uptr.reset(); }
    virtual void destroy_self() noexcept override { delete this; }
};

template <typename T, typename Alloc>
class local_control_block_resource final : public local_control_block_base, private /*EBO*/ Alloc {
    union {
        T m_obj;
    };

    using self_alloc_type = typename allocator_rebind<Alloc>::template to<local_control_block_resource>;

    static self_alloc_type get_self_alloc(const Alloc& a) {
        self_alloc_type myalloc = a;
        return myalloc;
    }
public:
    explicit local_control_block_resource(Alloc&& a) : Alloc(std::move(a)) {}
    ~local_control_block_resource() {}

    [[nodiscard]] static local_control_block_resource* create(Alloc a = {}) {
        auto myalloc = get_self_alloc(a);
        auto self = myalloc.allocate(1);
        new (self) local_control_block_resource(std::move(a));
        return self;
    }

    [[nodiscard]] T* ptr() {
        return &m_obj;
    }

    virtual void destroy_resource() noexcept override { m_obj.~T(); }
    virtual void destroy_self() noexcept override {
        self_alloc_type myalloc = get_self_alloc(*this); // slice
        this->~local_control_block_resource();
        myalloc.deallocate(this, 1);
    }
};

template <typename T>
using local_control_block_resource_dalloc = local_control_block_resource<T, allocator<char>>;

struct local_control_block {
    using cb_type = local_control_block_base;

    template <typename T, typename Del>
    [[nodiscard]] static cb_ptr_pair<cb_type, T> make_uptr_cb(unique_ptr<T, Del>& uptr) {
        auto cb = new local_control_block_uptr(uptr);
        return {cb, cb->ptr()};
    }

    template <typename T, typename... Args>
    [[nodiscard]] static cb_ptr_pair<cb_type, T> make_resource_cb(Args&&... args) {
        auto cb = local_control_block_resource_dalloc<T>::create();
        new (cb->ptr()) T(std::forward<Args>(args)...);
        return {cb, cb->ptr()};
    }

    template <typename T>
    [[nodiscard]] static cb_ptr_pair<cb_type, T> make_resource_cb_for_overwrite() {
        auto cb = local_control_block_resource_dalloc<T>::create();
        new (cb->ptr()) T;
        return {cb, cb->ptr()};
    }
};

}
