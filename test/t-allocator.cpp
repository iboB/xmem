// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <doctest/doctest.h>

#include <xmem/allocator.hpp>
#include <xmem/allocator_rebind.hpp>

#include <xmem/test_types.hpp>

#include <new>

TEST_SUITE_BEGIN("allocator");

TEST_CASE("basic") {
    static_assert(size_t(xmem::allocator<int>::align_val) == alignof(int));
    static_assert(size_t(xmem::allocator<obj>::align_val) == alignof(void*));
    static_assert(size_t(xmem::allocator<avx_512>::align_val) == 64);

    {
        // alloc-dealloc
        xmem::allocator<int> ia;
        constexpr size_t n = 1024;
        auto ints = ia.allocate(n);
        REQUIRE(ints);
        // this shouldn't crash
        for (int i = 0; i < int(n); ++i) {
            ints[i] = i;
        }
        CHECK(ints[1023] == 1023);
        ia.deallocate(ints, n);
    }

    {
        // no construct
        xmem::allocator<obj> oa;
        auto o = oa.allocate(1);
        REQUIRE(o);
        new (o) obj(5, "xyz"); // should be safe
        CHECK(o->a == 5);
        CHECK(o->b == "xyz");
        o->~obj();
        oa.deallocate(o, 1);
    }

    {
        // align
        xmem::allocator<avx_512> aa;
        auto a = aa.allocate(1);
        CHECK(reinterpret_cast<uintptr_t>(a) % 64 == 0);
        aa.deallocate(a, 1);
    }
}

TEST_CASE("convert/rebind") {
    // only test compilation
    xmem::allocator<char> a_char;

    using a_int_t = typename xmem::allocator_rebind<decltype(a_char)>::template to<int>;
    static_assert(std::is_same_v<a_int_t, xmem::allocator<int>>);

    a_int_t a_int = a_char;

    xmem::allocator<obj> a_obj = a_int;
}

