// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once

#include <atomic>

namespace xmem {

class atomic_control_block_base {
    std::atomic_uint32_t m_strong_refs = 1;
    std::atomic_uint32_t m_weak_refs = 1;
public:
    virtual ~atomic_control_block_base() noexcept = default;

    virtual void destroy_resource() noexcept = 0;
    virtual void destroy_self() noexcept = 0;

    void inc_strong_ref() noexcept {
        m_strong_refs.fetch_add(1, std::memory_order_relaxed);
    }
    void dec_strong_ref() noexcept {
        if (m_weak_refs.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            destroy_resource();
            dec_weak_ref();
        }
    }
    uint32_t inc_strong_ref_nz() noexcept {
        auto rc = m_strong_refs.load(std::memory_order_acquire);
        while (rc != 0) {
            if (m_strong_refs.compare_exchange_weak(rc, rc + 1, std::memory_order_acq_rel, std::memory_order_relaxed)) {
                return rc + 1;
            }
        }
        return 0;
    }

    void inc_weak_ref() noexcept {
        m_weak_refs.fetch_add(1, std::memory_order_relaxed);
    }
    void dec_weak_ref() noexcept {
        if (m_weak_refs.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            destroy_self();
        }
    }

    long strong_ref_count() const noexcept {
        return long(m_strong_refs.load(std::memory_order_relaxed);
    }
};

}
