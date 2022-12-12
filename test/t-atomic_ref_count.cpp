// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <doctest/doctest.h>

#include <xmem/atomic_ref_count.hpp>

TEST_SUITE_BEGIN("atomic_ref_count");

using ref_count_type = xmem::atomic_ref_count;

#include "test_ref_count.inl"