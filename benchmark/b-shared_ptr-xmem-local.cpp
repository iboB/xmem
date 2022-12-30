// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <xmem/local_shared_ptr.hpp>

#define FUNC xmem_local_sptr
#define sptr xmem::local_shared_ptr
#define wptr xmem::local_weak_ptr
#define make xmem::make_local_shared

#include "b-shared_ptr.inl"
PICOBENCH(xmem_local_sptr);
