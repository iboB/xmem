// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <cstdint>

namespace xmem {

class local_ref_count {
    uint32_t m_refs = 1;
public:
    uint32_t inc() noexcept { return ++m_refs; }
    uint32_t dec() noexcept { return --m_refs; }
    uint32_t count() const noexcept { return m_refs; }
    uint32_t inc_nz() noexcept {
        if (m_refs == 0) return 0;
        return inc();
    }
};

}