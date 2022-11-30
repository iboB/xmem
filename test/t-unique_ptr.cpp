// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <doctest/doctest.h>

#include <xmem/unique_ptr.hpp>
#include <doctest/util/lifetime_counter.hpp>

#include <vector>

TEST_SUITE_BEGIN("unique_ptr");

struct obj : public doctest::util::lifetime_counter<obj> {
    obj() = default;
    obj(int a, std::string b) : a(a), b(b) {}

    int a = 11;
    std::string b;
};

TEST_CASE("basic") {
    using iptr = xmem::unique_ptr<int>;
    static_assert(sizeof(iptr) == sizeof(int*));
    static_assert(std::is_same_v<iptr::pointer, int*>);
    static_assert(std::is_same_v<iptr::element_type, int>);
    static_assert(std::is_same_v<iptr::deleter_type, xmem::default_delete<int>>);

    using optr = xmem::unique_ptr<obj>;
    static_assert(sizeof(optr) == sizeof(obj*));
    static_assert(std::is_same_v<optr::pointer, obj*>);
    static_assert(std::is_same_v<optr::element_type, obj>);
    static_assert(std::is_same_v<optr::deleter_type, xmem::default_delete<obj>>);

    obj::lifetime_stats stats;

    {
        optr e;
        CHECK_FALSE(e);
        CHECK_FALSE(e.get());
        CHECK_FALSE(e.release());
        e.reset(); // should be safe
        e = nullptr;
        CHECK_FALSE(e);
    }

    CHECK(stats.total == 0);

    {
        int i = 10;
        iptr p(&i); // kinda hacky, but we promise we'll release
        CHECK(p);
        CHECK(p.get() == &i);
        CHECK(*p == 10);
        CHECK(p.release() == &i);
        CHECK_FALSE(p.release());
        p = nullptr;
        CHECK_FALSE(p);
    }

    {
        optr o;
        o.reset(new obj(2, "xxx"));
        CHECK(o);
        CHECK(o->a == 2);
        CHECK(o->b == "xxx");
        o.reset();
        CHECK_FALSE(o);

        CHECK(stats.total == 1);
        CHECK(stats.living == 0);
    }
}

TEST_CASE("make_unique") {
    obj::lifetime_stats stats;

    {
        auto uptr = xmem::make_unique<obj>(1, "ads");
        CHECK(uptr);
        CHECK(uptr->a == 1);
        CHECK(uptr->b == "ads");

        auto p = uptr.get();

        auto uptr2 = std::move(uptr);
        CHECK_FALSE(uptr);
        CHECK(uptr2);
        CHECK(uptr2.get() == p);

        CHECK(stats.d_ctr == 1);
        CHECK(stats.living == 1);
    }

    CHECK(stats.living == 0);
    CHECK(stats.total == 1);
}


TEST_CASE("make_unique_ptr") {
    std::vector<int> vec = {1, 2, 3};
    auto copy = xmem::make_unique_ptr(vec);
    CHECK(copy->size() == 3);
    CHECK(vec.size() == 3);
    CHECK(vec.data() != copy->data());
    copy->at(1) = 5;
    CHECK(*copy == std::vector<int>({1, 5, 3}));
    auto vdata = vec.data();
    auto heist = xmem::make_unique_ptr(std::move(vec));
    CHECK(heist->size() == 3);
    CHECK(heist->data() == vdata);
}
