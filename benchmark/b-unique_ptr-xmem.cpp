// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <picobench/picobench.hpp>
#include <xmem/unique_ptr.hpp>
#include <random>

namespace xmem {
#include "b-unique_ptr.inl"
}

PICOBENCH(xmem::uptr);
