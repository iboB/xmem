// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#define XMEM_TEST_NAMESPACE std
#define ENABLE_XMEM_SPECIFIC_CHECKS 0
#include <xmem/test_init.inl>

#include <memory>
#include <doctest/doctest.h>
TEST_SUITE_BEGIN("sanity std::unique_ptr");

#include <xmem/test-unique_ptr.inl>
