// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "allocator_rebind.hpp"
#include "basic_shared_from.hpp"
#include "allocator.hpp"

namespace xmem {

template <typename RC>
class control_block_base {
    RC m_strong;
    RC m_weak;
public:
    void inc_strong_ref(const void*) noexcept {
        m_strong.inc();
    }
    void dec_strong_ref(const void* src) noexcept {
        if (m_strong.dec() == 0) {
            destroy_resource();
            dec_weak_ref(src);
        }
    }
    bool inc_strong_ref_nz(const void*) noexcept {
        return !!m_strong.inc_nz();
    }
    long strong_ref_count() const noexcept {
        return long(m_strong.count());
    }
    void transfer_strong(const void*, const void*) {}

    void inc_weak_ref(const void*) noexcept {
        m_weak.inc();
    }
    void dec_weak_ref(const void*) noexcept {
        if (m_weak.dec() == 0) {
            destroy_self();
        }
    }
    void transfer_weak(const void*, const void*) {}

protected:
    virtual void destroy_resource() noexcept = 0;
    virtual void destroy_self() noexcept = 0;
};

template <typename Base, typename T, typename Alloc>
class control_block_resource final : public Base, private /*EBO*/ Alloc {
    union {
        T m_obj;
    };

    using self_alloc_type = typename allocator_rebind<Alloc>::template to<control_block_resource>;

    static self_alloc_type get_self_alloc(const Alloc& a) {
        self_alloc_type myalloc = a;
        return myalloc;
    }
public:
    explicit control_block_resource(Alloc&& a) : Alloc(std::move(a)) {}
    ~control_block_resource() {}

    using control_block_resource_ptr = unique_ptr<control_block_resource, void(*)(control_block_resource*)>;
    [[nodiscard]] static control_block_resource_ptr create(Alloc a) {
        auto myalloc = get_self_alloc(a);
        auto self = myalloc.allocate(1);
        new (self) control_block_resource(std::move(a));
        return control_block_resource_ptr(self, [](control_block_resource* ptr) { ptr->destroy_self(); });
    }

    [[nodiscard]] T* obj() {
        return &m_obj;
    }

    virtual void destroy_resource() noexcept override { m_obj.~T(); }
    virtual void destroy_self() noexcept override {
        self_alloc_type myalloc = get_self_alloc(*this); // slice
        this->~control_block_resource();
        myalloc.deallocate(this, 1);
    }
};

template <typename CB>
struct control_block_factory {
    using cb_type = CB;

    template <typename T>
    using pair = cb_ptr_pair<cb_type, T>;

    template <typename T>
    using sptr = basic_shared_ptr<control_block_factory, T>;

    template <typename T>
    static pair<T> prepare_pair(cb_type* cb, T* ptr) noexcept {
        using ebf_type = basic_enable_shared_from<control_block_factory>;
        if constexpr (std::is_base_of_v<ebf_type, T>) {
            ebf_type* ebf = ptr;
            ebf->m.cb = cb;
            ebf->m.ptr = ptr;
        }
        return {cb, ptr};
    }

    template <typename T, typename Del, typename Alloc = allocator<char>>
    [[nodiscard]] static pair<T> make_uptr_cb(unique_ptr<T, Del>& uptr, Alloc a = {}) {
        using uptr_type = unique_ptr<T, Del>;
        using rsrc_type = control_block_resource<cb_type, uptr_type, Alloc>;
        auto tmp = rsrc_type::create(std::move(a));
        new (tmp->obj()) uptr_type(std::move(uptr));
        auto cb = tmp.release();
        return prepare_pair(cb, cb->obj()->get());
    }

    template <typename T, typename Alloc, typename... Args>
    [[nodiscard]] static pair<T> make_resource_cb(Alloc a, Args&&... args) {
        using rsrc_type = control_block_resource<cb_type, T, Alloc>;
        auto tmp = rsrc_type::create(std::move(a));
        new (tmp->obj()) T(std::forward<Args>(args)...);
        auto cb = tmp.release();
        return prepare_pair(cb, cb->obj());
    }

    template <typename T, typename Alloc>
    [[nodiscard]] static pair<T> make_resource_cb_for_overwrite(Alloc a) {
        using rsrc_type = control_block_resource<cb_type, T, Alloc>;
        auto tmp = rsrc_type::create(std::move(a));
        new (tmp->obj()) T;
        auto cb = tmp.release();
        return prepare_pair(cb, cb->obj());
    }
};


}
