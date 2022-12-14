// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#define XMEM_TEST_NAMESPACE xmem
#define ENABLE_XMEM_SPECIFIC_CHECKS 1
#include <xmem/test_init.inl>

#include <xmem/unique_ptr.hpp>
#include <doctest/doctest.h>
TEST_SUITE_BEGIN("unique_ptr");

#include <xmem/test-unique_ptr.inl>
