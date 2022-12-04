// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <doctest/doctest.h>

#include <xmem/allocator.hpp>
#include <xmem/allocator_rebind.hpp>

#include <xmem/test_types.hpp>

#include <memory>

TEST_SUITE_BEGIN("allocator compatibility with std::");

TEST_CASE("xmem traits") {
    using xmt = std::allocator_traits<xmem::allocator<int>>;
    xmem::allocator<int> a;
    auto i = xmt::allocate(a, 1);
    REQUIRE(i);
    xmt::deallocate(a, i, 1);
}

TEST_CASE("rebind std") {
    // only test compilation
    std::allocator<char> a_char;

    using a_int_t = typename xmem::allocator_rebind<decltype(a_char)>::template to<int>;
    static_assert(std::is_same_v<a_int_t, std::allocator<int>>);

    a_int_t a_int = a_char;

    std::allocator<obj> a_obj = a_int;
}
