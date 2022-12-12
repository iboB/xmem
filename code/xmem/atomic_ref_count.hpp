// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <atomic>

namespace xmem {

class atomic_ref_count {
    std::atomic_uint32_t m_refs = {1};
public:
    uint32_t inc() noexcept {
        return m_refs.fetch_add(1, std::memory_order_relaxed) + 1;
    }
    uint32_t dec() noexcept {
        return m_refs.fetch_sub(1, std::memory_order_acq_rel) - 1;
    }
    uint32_t count() const noexcept {
        return m_refs.load(std::memory_order_relaxed);
    }
    uint32_t inc_nz() noexcept {
        auto rc = m_refs.load(std::memory_order_acquire);
        while (rc != 0) {
            if (m_refs.compare_exchange_weak(rc, rc + 1, std::memory_order_acq_rel, std::memory_order_relaxed)) {
                return rc + 1;
            }
        }
        return 0;
    }
};

}
