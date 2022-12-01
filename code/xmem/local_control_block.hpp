// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "unique_ptr.hpp"

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

    virtual void destroy_resource() noexcept override { m_uptr.reset(); }
    virtual void destroy_self() noexcept override { delete this; }
};

struct local_control_block {
    using base_type = local_control_block_base;

    template <typename T, typename D>
    using uptr_type = local_control_block_uptr<T, D>;
};

}
