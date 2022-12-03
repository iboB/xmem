// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "cb_ptr_pair.hpp"
#include "unique_ptr.hpp"
#include "allocator_rebind.hpp"

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
    local_control_block_uptr(uptr_t& uptr) : m_uptr(std::move(uptr)) {}

    T* ptr() {
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
public:
    template <typename... Args>
    local_control_block_resource(Alloc a, Args&&... args) : Alloc(std::move(a)) {
        new (&m_obj) T(std::forward<Args>(args)...);
    }

    T* ptr() {
        return &m_obj;
    }

    virtual void destroy_resource() noexcept override { m_obj.~T(); }
    virtual void destroy_self() noexcept override {
        using self_alloc_type = typename allocator_rebind<Alloc>::template to<local_control_block_resource>;
        self_alloc_type myalloc = (Alloc&)*this; // slice
        this->~local_control_block_base();
        myalloc.deallocate(this);
    }
};

struct local_control_block {
    using cb_type = local_control_block_base;

    template <typename T, typename Del>
    static cb_ptr_pair<cb_type, T> make_uptr_cb(unique_ptr<T, Del>& uptr) {
        auto cb = new local_control_block_uptr(uptr);
        return {cb, cb->ptr()};
    }

    template <typename T, typename A>
    using rsrc_type = local_control_block_resource<T, A>;
};

}
