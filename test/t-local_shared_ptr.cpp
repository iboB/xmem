// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <doctest/doctest.h>

#include <xmem/local_shared_ptr.hpp>
#include <doctest/util/lifetime_counter.hpp>

#include <vector>

TEST_SUITE_BEGIN("local_shared_ptr");

struct obj : public doctest::util::lifetime_counter<obj> {
    int a;
    explicit obj(int a = 0) : a(a) {}
    virtual ~obj() = default;
    virtual int val() const { return a; }
};

struct child : obj {
    int b;
    child(int a, int b) : obj(a), b(b) {}
    virtual int val() const override { return a + b; }
};

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
