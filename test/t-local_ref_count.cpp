// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <doctest/doctest.h>

#include <xmem/local_ref_count.hpp>

TEST_SUITE_BEGIN("local_ref_count");

using ref_count_type = xmem::local_ref_count;

#include "test_ref_count.inl"
