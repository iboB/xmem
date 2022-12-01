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
        e.reset(); // should be safe
        e = nullptr;
        CHECK_FALSE(e);
    }

    CHECK(stats.total == 0);

    {
        sptr e(new obj(44));
        CHECK(e);
        CHECK(e.get());
        CHECK(e.get()->a == 44);
        CHECK(e->a == 44);
    }

    CHECK(stats.total == 1);
    CHECK(stats.living == 0);
}
