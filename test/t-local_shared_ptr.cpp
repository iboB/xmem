// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#define XMEM_TEST_NAMESPACE xmem
#define ENABLE_XMEM_SPECIFIC_CHECKS 1
#include <xmem/test_init.inl>

#include <xmem/local_shared_ptr.hpp>
#include <doctest/doctest.h>
TEST_SUITE_BEGIN("local_shared_ptr");

#define test_shared_ptr local_shared_ptr
#define make_test_shared make_local_shared
#define make_test_shared_ptr make_local_shared_ptr
#define make_test_shared_for_overwrite make_local_shared_for_overwrite

#include <xmem/test-shared_ptr-local.inl>

#define test_weak_ptr local_weak_ptr
#define enable_test_shared_from enable_local_shared_from
#define enable_test_shared_from_this enable_local_shared_from_this

#include <xmem/test-weak_ptr-shared_from-local.inl>

