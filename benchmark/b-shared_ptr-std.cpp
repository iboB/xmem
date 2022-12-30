// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <memory>

#define FUNC std_sptr
#define sptr std::shared_ptr
#define wptr std::weak_ptr
#define make std::make_shared

#include "b-shared_ptr.inl"
PICOBENCH(std_sptr);
