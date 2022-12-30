// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <xmem/shared_ptr.hpp>

#define FUNC xmem_sptr
#define sptr xmem::shared_ptr
#define wptr xmem::weak_ptr
#define make xmem::make_shared

#include "b-shared_ptr.inl"
PICOBENCH(xmem_sptr);
