// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#define XMEM_TEST_NAMESPACE xmem
#define ENABLE_XMEM_SPECIFIC_CHECKS 1
#include <xmem/test_init.inl>

#include <xmem/shared_ptr.hpp>
#include <doctest/doctest.h>
TEST_SUITE_BEGIN("shared_ptr");

#define test_shared_ptr shared_ptr
#define make_test_shared make_shared
#define make_test_shared_ptr make_shared_ptr
#define make_test_shared_for_overwrite make_shared_for_overwrite

#include <xmem/test-shared_ptr-local.inl>

#define test_weak_ptr weak_ptr
#define enable_test_shared_from enable_shared_from
#define enable_test_shared_from_this enable_shared_from_this

#include <xmem/test-weak_ptr-shared_from-local.inl>

#include <xmem/test-shared_ptr-atomic.inl>
#include <xmem/test-weak_ptr-atomic.inl>

