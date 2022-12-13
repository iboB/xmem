// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#define XMEM_TEST_NAMESPACE std
#define ENABLE_XMEM_SPECIFIC_CHECKS 0
#include <xmem/test_init.inl>

#include <memory>
#include <doctest/doctest.h>
TEST_SUITE_BEGIN("local_shared_ptr");

#define test_shared_ptr shared_ptr
#define make_test_shared make_shared
#define make_test_shared_ptr make_shared_ptr
#define make_test_shared_for_overwrite make_shared_for_overwrite

#include <xmem/test-shared_ptr-basic.inl>

#define test_weak_ptr weak_ptr
#define enable_test_shared_from_this enable_shared_from_this

#include <xmem/test-weak_ptr-shared_from-basic.inl>

