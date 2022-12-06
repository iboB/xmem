// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <doctest/doctest.h>

#include <xmem/local_shared_ptr.hpp>

#define XMEM_TEST_NAMESPACE xmem
#define ENABLE_XMEM_SPECIFIC_CHECKS 1
#include <xmem/test_init.inl>

TEST_SUITE_BEGIN("local_shared_ptr");

TEST_CASE("shared_ptr: basic") {
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

TEST_CASE("shared_ptr: compare/swap") {
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

TEST_CASE("make_shared") {
    obj::lifetime_stats stats;

    {
        auto sptr = xmem::make_local_shared<obj>();
        CHECK(sptr->a == 11);
        CHECK(sptr->b.empty());
    }

    {
        auto sptr = xmem::make_local_shared<obj>(1, "abc");
        CHECK(sptr->a == 1);
        CHECK(sptr->b == "abc");

        auto s2 = sptr;
        auto s3 = sptr;

        CHECK(sptr.use_count() == 3);
        CHECK(stats.living == 1);
    }

    CHECK(stats.total == 2);
}

STD20(TEST_CASE("make_shared_for_overwrite") {
    // should compile
    auto ip = xmem::make_local_shared_for_overwrite<int>();
    CHECK(ip);

    auto op = xmem::make_local_shared_for_overwrite<obj>();
    CHECK(op->a == 11);
})

XMEM(TEST_CASE("make_shared_ptr") {
    std::vector<int> vec = {1, 2, 3};
    auto copy = test::make_local_shared_ptr(vec);
    CHECK(copy->size() == 3);
    CHECK(vec.size() == 3);
    CHECK(vec.data() != copy->data());
    copy->at(1) = 5;
    CHECK(*copy == std::vector<int>({1, 5, 3}));
    auto vdata = vec.data();
    auto heist = test::make_local_shared_ptr(std::move(vec));
    CHECK(heist->size() == 3);
    CHECK(heist->data() == vdata);
})

TEST_CASE("shared_ptr: cast and type erasure") {
    obj::lifetime_stats stats;

    auto c = xmem::make_local_shared<child>(1, 2);

    xmem::local_shared_ptr<obj> p = c;
    CHECK(p);
    CHECK(p.use_count() == 2);
    CHECK(p->val() == 3);

    {
        xmem::local_shared_ptr<const obj> cp = c;
        CHECK(cp);
        CHECK(cp.use_count() == 3);
        CHECK(cp->val() == 3);
    }

    xmem::local_shared_ptr<void> v = p;
    p.reset();
    c.reset();

    CHECK(v.use_count() == 1);

    auto rp = reinterpret_cast<obj*>(v.get());
    CHECK(rp->val() == 3);
}

TEST_CASE("shared_ptr: alias") {
    obj::lifetime_stats stats;

    auto c = xmem::make_local_shared<child>(10, 20);

    auto i = xmem::local_shared_ptr<int>(c, &c->a);
    auto i2 = xmem::local_shared_ptr<int>(c, &c->c);

    CHECK(i.use_count() == 3);

    c.reset();

    CHECK(i2.use_count() == 2);

    CHECK(*i == 10);
    CHECK(*i2 == 20);
}

//////////////////////////////////////////////////////////

TEST_CASE("weak_ptr basic") {
    using sptr = xmem::local_shared_ptr<obj>;
    using wptr = xmem::local_weak_ptr<obj>;

    obj::lifetime_stats stats;
    doctest::util::lifetime_counter_sentry _s(stats);

    {
        wptr e;
        XMEM(CHECK_FALSE(e));
        XMEM(CHECK_FALSE(e.owner()));
        CHECK(e.use_count() == 0);
        CHECK(e.expired());
        CHECK_FALSE(e.lock());

        wptr e2 = e;
        XMEM(CHECK_FALSE(e2));
        XMEM(CHECK_FALSE(e2.owner()));
        CHECK(e2.use_count() == 0);
        CHECK(e2.expired());
        CHECK_FALSE(e2.lock());
    }

    CHECK(stats.total == 0);

    {
        sptr se;
        wptr we = se;
        XMEM(CHECK_FALSE(we));
        XMEM(CHECK_FALSE(we.owner()));
        CHECK(we.use_count() == 0);
        CHECK(se.use_count() == 0);
        CHECK_FALSE(we.lock());
        CHECK(we.expired());
    }

    CHECK(stats.total == 0);

    {
        auto se = xmem::make_local_shared<child>(5, 10);
        wptr w = se;
        CHECK(w.use_count() == 1);
        CHECK(se.use_count() == 1);
        XMEM(CHECK(w));
        XMEM(CHECK(w.owner() == se.owner()));
        wptr w2 = w;
        XMEM(CHECK(w2.owner() == se.owner()));
        CHECK(w2.lock() == se);
        sptr se2 = se;
        CHECK(w.use_count() == 2);

        se.reset();
        se2.reset();
        CHECK(stats.living == 0);

        CHECK(w.use_count() == 0);
        CHECK(w.expired());
        XMEM(CHECK(w));
        XMEM(CHECK(w.owner()));

        w.reset();
        XMEM(CHECK_FALSE(w));
        XMEM(CHECK_FALSE(w.owner()));

        CHECK_FALSE(w2.lock());
        XMEM(CHECK(w2));
        XMEM(CHECK(w2.owner()));
    }

    {
        auto sp = xmem::make_local_shared<obj>(5, "xyz");
        {
            wptr wp = sp;
            CHECK_FALSE(wp.expired());
            CHECK(wp.use_count() == 1);
        }
        CHECK(sp->a == 5);
        CHECK(sp->b == "xyz");
    }

    CHECK(stats.total == 2);
    CHECK(stats.living == 0);
}
