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

struct parent : public doctest::util::lifetime_counter<parent> {
    int a;
    explicit parent(int a = 0) : a(a) {}
    virtual ~parent() = default;
    virtual int val() const { return a; }
};

struct child : parent {
    int b;
    child(int a, int b) : parent(a), b(b) {}
    virtual int val() const override { return a + b; }
};

struct test_deleter {
    intptr_t dels = 0;
    void operator()(int* iptr) {
        ++dels;
        delete iptr;
    }
    void operator()(obj* optr) {
        ++dels;
        delete optr;
    }

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
        *p = 5;
        CHECK(*p == 5);
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

TEST_CASE("compare / swap") {
    int vals[] = {1, 2};

    using iptr = xmem::unique_ptr<int>;
    iptr p0(vals + 0);
    iptr p0a(vals + 0);
    iptr p1(vals + 1);

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

    CHECK(p0.release() == vals + 1);
    CHECK(p0a.release() == vals + 0);
    CHECK(p1.release() == vals + 0);
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

TEST_CASE("template move") {
    parent::lifetime_stats stats;

    {
        auto c = xmem::make_unique<child>(1, 2);
        xmem::unique_ptr<parent> p = std::move(c);
        CHECK_FALSE(c);
        CHECK(p);
        CHECK(p->a == 1);
        CHECK(p->val() == 3);
    }

    CHECK(stats.living == 0);
    CHECK(stats.total == 1);

    {
        auto p = xmem::make_unique<parent>(5);
        auto c = xmem::make_unique<child>(10, 20);
        p = std::move(c);
        CHECK(p->val() == 30);
    }

    CHECK(stats.living == 0);
    CHECK(stats.total == 3);
}

TEST_CASE("make_unique_for_overwrite") {
    // should compile
    auto ip = xmem::make_unique_for_overwrite<int>();
    CHECK(ip);

    auto op = xmem::make_unique_for_overwrite<obj>();
    CHECK(op->a == 11);
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

TEST_CASE("deleter") {
    using optr = xmem::unique_ptr<obj, test_deleter>;
    static_assert(sizeof(optr) == sizeof(obj*) + sizeof(intptr_t));
    static_assert(std::is_same_v<optr::pointer, obj*>);
    static_assert(std::is_same_v<optr::element_type, obj>);
    static_assert(std::is_same_v<optr::deleter_type, test_deleter>);

    obj::lifetime_stats stats;

    {
        optr e;
        e.reset();
        CHECK(e.get_deleter().dels == 0);
    }

    {
        optr o(new obj(44, "x"));
        o.reset();
        CHECK(o.get_deleter().dels == 1);

        o.reset(new obj(21, "xy"));
        optr o2(new obj(33, "xyz"));
        o = std::move(o2);
        CHECK(o.get_deleter().dels == 0);
        CHECK(o2.get_deleter().dels == 0);

        CHECK(stats.living == 1);
        CHECK(stats.total == 3);
    }

    CHECK(stats.living == 0);
    CHECK(stats.total == 3);
}

TEST_CASE("ref deleter") {
    using iptr = xmem::unique_ptr<int, test_deleter&>;
    static_assert(sizeof(iptr) == sizeof(int*) + sizeof(test_deleter*));
    static_assert(std::is_same_v<iptr::pointer, int*>);
    static_assert(std::is_same_v<iptr::element_type, int>);
    static_assert(std::is_same_v<iptr::deleter_type, test_deleter&>);

    test_deleter d;
    {
        iptr e(d);
        e.reset();
        CHECK(&e.get_deleter() == &d);
    }
    CHECK(d.dels == 0);

    {
        iptr i(new int(35), d);
        CHECK(*i == 35);
        *i = 10;
        CHECK(*i == 10);
        auto i2 = std::move(i);
        CHECK(!i);
    }
    CHECK(d.dels == 1);
}

TEST_CASE("func deleter") {
    using func = void(*)(int*);
    using iptr = xmem::unique_ptr<int, func>;
    static_assert(sizeof(iptr) == sizeof(int*) + sizeof(func));
    static_assert(std::is_same_v<iptr::pointer, int*>);
    static_assert(std::is_same_v<iptr::element_type, int>);
    static_assert(std::is_same_v<iptr::deleter_type, func>);

    static int dels = 0;
    dels = 0;
    auto deleter = [](int* p) {
        delete p;
        ++dels;
    };
    {
        iptr e(deleter);
        e.reset();
    }
    CHECK(dels == 0);

    {
        iptr i(new int(35), deleter);
        CHECK(*i == 35);
        *i = 10;
        CHECK(*i == 10);
        auto i2 = std::move(i);
        CHECK(!i);
    }
    CHECK(dels == 1);
}
