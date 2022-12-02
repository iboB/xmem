// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <doctest/doctest.h>

#include <xmem/local_shared_ptr.hpp>

#define XMEM_TEST_NAMESPACE xmem
#define ENABLE_XMEM_SPECIFIC_CHECKS 1
#include <xmem/test_init.inl>

TEST_SUITE_BEGIN("local_shared_ptr");

TEST_CASE("basic") {
    using sptr = xmem::local_shared_ptr<obj>;

    obj::lifetime_stats stats;

    {
        sptr e;
        CHECK_FALSE(e);
        CHECK_FALSE(e.get());
        CHECK(e.use_count() == 0);
        CHECK_FALSE(e.owner());
        e.reset(); // should be safe
        e = nullptr;
        CHECK_FALSE(e);
    }

    CHECK(stats.total == 0);

    {
        sptr o(new obj(44));
        CHECK(o);
        CHECK(o.use_count() == 1);
        CHECK(o.owner());
        CHECK(o.get());
        CHECK(o.get()->a == 44);
        CHECK(o->a == 44);
    }

    CHECK(stats.total == 1);
    CHECK(stats.living == 0);

    {
        sptr o(new obj(11));
        auto p = o.get();
        auto oo = o.owner();
        CHECK(oo);
        sptr mo = std::move(o);
        CHECK_FALSE(o);
        CHECK(o.get() == 0);
        CHECK(o.use_count() == 0);
        CHECK_FALSE(o.owner());
        CHECK(mo);
        CHECK(mo.get() == p);
        CHECK(mo.owner() == oo);
        CHECK(mo.use_count() == 1);

        CHECK(stats.living == 1);
    }

    CHECK(stats.total == 2);
    CHECK(stats.living == 0);

    {
        sptr u(new obj(10));
        CHECK(stats.living == 1);
        sptr o(new obj(53));
        CHECK(stats.living == 2);

        CHECK(o.owner() != u.owner());

        auto o2 = o;
        CHECK(o2.get() == o.get());
        CHECK(o2.owner() == o.owner());
        CHECK(o2->a == 53);
        CHECK(o.use_count() == 2);
        auto o3 = o2;
        CHECK(o3.get() == o.get());
        CHECK(o3.owner() == o.owner());
        CHECK(o3->a == 53);
        CHECK(o.use_count() == 3);
        CHECK(stats.living == 2);
        o.reset();
        CHECK_FALSE(o);
        CHECK(o.use_count() == 0);
        CHECK(o2.use_count() == 2);
        CHECK(stats.living == 2);
        o2.reset();
        CHECK_FALSE(o2);
        CHECK(o3->a == 53);
        CHECK(o3.use_count() == 1);
        CHECK(stats.living == 2);
        o3.reset();
        CHECK(stats.living == 1);
        CHECK(stats.total == 4);
    }
}

TEST_CASE("basic compare/swap") {
    int vals[] = {1, 2};

    using uiptr = xmem::unique_ptr<int, void(*)(int*)>;
    uiptr up0(vals + 0, [](int*) {});
    uiptr up1(vals + 1, [](int*) {});

    using siptr = xmem::local_shared_ptr<int>;
    siptr p0(std::move(up0));
    CHECK(p0 == p0);
    CHECK_FALSE(p0 != p0);
    CHECK_FALSE(p0 < p0);
    CHECK_FALSE(p0 > p0);
    CHECK(p0 >= p0);
    CHECK(p0 <= p0);

    siptr p0a = p0;
    siptr p1(std::move(up1));

    CHECK(p0 == p0a);
    CHECK(p1 == p1);
    CHECK_FALSE(p0 == p1);
    CHECK(p0 != p1);
    CHECK_FALSE(p0 != p0a);
    CHECK(p0 < p1);
    CHECK_FALSE(p1 < p0);
    CHECK(p0 <= p1);
    CHECK(p0 <= p0a);
    CHECK_FALSE(p1 <= p0);
    CHECK(p1 > p0);
    CHECK_FALSE(p0 > p1);
    CHECK(p1 >= p0);
    CHECK(p0a >= p0);
    CHECK_FALSE(p0 >= p1);

    p1.swap(p0);
    CHECK(p0.get() == vals + 1);
    CHECK(p1.get() == vals + 0);
    CHECK(p1 < p0);
}
