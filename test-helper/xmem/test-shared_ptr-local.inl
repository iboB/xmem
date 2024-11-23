// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//

// inline file - no include guard

// shared_ptr basic test

TEST_CASE("shared_ptr: basic") {
    using sptr = test::test_shared_ptr<obj>;

    obj::lifetime_stats stats;

    {
        sptr e;
        CHECK_FALSE(e);
        CHECK_FALSE(e.get());
        CHECK(e.use_count() == 0);
        XMEM(CHECK_FALSE(e.owner()));
        CHECK(xtest::no_owner(e));
        e.reset(); // should be safe
        e = nullptr;
        CHECK_FALSE(e);
    }

    CHECK(stats.total == 0);

    {
        sptr o(new obj(44));
        CHECK(o);
        CHECK(o.use_count() == 1);
        XMEM(CHECK(o.owner()));
        CHECK_FALSE(xtest::no_owner(o));
        CHECK(o.get());
        CHECK(o.get()->a == 44);
        CHECK(o->a == 44);
    }

    CHECK(stats.total == 1);
    CHECK(stats.living == 0);

    {
        sptr o(new obj(11));
        auto p = o.get();
        XMEM(auto oo = o.owner());
        XMEM(CHECK(oo));
        sptr mo = std::move(o);
        CHECK_FALSE(o);
        CHECK(o.get() == 0);
        CHECK(o.use_count() == 0);
        XMEM(CHECK_FALSE(o.owner()));
        CHECK(xtest::no_owner(o));
        CHECK(mo);
        CHECK(mo.get() == p);
        XMEM(CHECK(mo.owner() == oo));
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

        XMEM(CHECK(o.owner() != u.owner()));
        XMEM(CHECK(o.t_owner() != u.t_owner()));
        CHECK_FALSE(xtest::same_owner(o, u));

        auto o2 = o;
        CHECK(o2.get() == o.get());
        XMEM(CHECK(o2.owner() == o.owner()));
        CHECK(xtest::same_owner(o2, o));
        CHECK(o2->a == 53);
        CHECK(o.use_count() == 2);
        auto o3 = o2;
        CHECK(o3.get() == o.get());
        XMEM(CHECK(o3.owner() == o.owner()));
        CHECK(xtest::same_owner(o3, o));
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

    using uiptr = test::unique_ptr<int, void(*)(int*)>;
    uiptr up0(vals + 0, [](int*) {});
    uiptr up1(vals + 1, [](int*) {});

    using siptr = test::test_shared_ptr<int>;
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

    siptr e1;
    siptr e2;
    e1.swap(e2);
    CHECK_FALSE(e1);
    CHECK_FALSE(e2);
}

TEST_CASE("make_shared") {
    obj::lifetime_stats stats;

    {
        auto sptr = test::make_test_shared<obj>();
        CHECK(sptr->a == 11);
        CHECK(sptr->b.empty());
    }

    {
        auto sptr = test::make_test_shared<obj>(1, "abc");
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
    auto ip = test::make_test_shared_for_overwrite<int>();
    CHECK(ip);

    auto op = test::make_test_shared_for_overwrite<obj>();
    CHECK(op->a == 11);
})

TEST_CASE("make_shared_ptr") {
    std::vector<int> vec = {1, 2, 3};
    auto copy = xtest::make_test_shared_ptr(vec);
    CHECK(copy->size() == 3);
    CHECK(vec.size() == 3);
    CHECK(vec.data() != copy->data());
    copy->at(1) = 5;
    CHECK(*copy == std::vector<int>({1, 5, 3}));
    auto vdata = vec.data();
    auto heist = xtest::make_test_shared_ptr(std::move(vec));
    CHECK(heist->size() == 3);
    CHECK(heist->data() == vdata);
}

TEST_CASE("shared_ptr: cast and type erasure") {
    obj::lifetime_stats stats;

    auto c = test::make_test_shared<child>(1, 2);

    test::test_shared_ptr<obj> p = c;
    CHECK(p);
    CHECK(p.use_count() == 2);
    CHECK(p->val() == 3);

    {
        test::test_shared_ptr<const obj> cp = c;
        CHECK(cp);
        CHECK(cp.use_count() == 3);
        CHECK(cp->val() == 3);
    }

    test::test_shared_ptr<void> v = p;
    p.reset();
    c.reset();

    CHECK(v.use_count() == 1);

    auto rp = reinterpret_cast<obj*>(v.get());
    CHECK(rp->val() == 3);
}

TEST_CASE("shared_ptr: alias") {

    // make-aliased non-null
    {
        auto ptr = xtest::make_test_shared_ptr(vec{1, 2});
        auto alias = xtest::make_aliased(ptr, &ptr->y);
        CHECK(alias);
        CHECK(*alias == 2);
        CHECK(alias.use_count() == 2);
    }

    // make-aliased null
    {
        test::test_shared_ptr<vec> ptr;
        auto alias = xtest::make_aliased(ptr, &ptr->y);
        CHECK_FALSE(alias);
        CHECK(alias.use_count() == 0);
    }


    auto c = test::make_test_shared<child>(10, 20);

    auto i = test::test_shared_ptr<int>(c, &c->a);
    CHECK(i.get() == &c->a);
    XMEM(CHECK(i.owner() == c.owner()));
    CHECK(xtest::same_owner(i, c));

    auto i2 = test::test_shared_ptr<int>(c, &c->c);
    CHECK(i2.get() == &c->c);
    XMEM(CHECK(i2.owner() == i.owner()));
    XMEM(CHECK(i2.t_owner() == i.t_owner()));
    CHECK(xtest::same_owner(i2, i));

    CHECK(i.use_count() == 3);

    i2.reset();
    CHECK_FALSE(i2);
    i2 = xtest::make_aliased(c, &c->c);
    CHECK(i2);

    CHECK(*i2 == 20);

    CHECK(i.use_count() == 3);

    c.reset();

    CHECK(i2.use_count() == 2);

    CHECK(*i == 10);
    CHECK(*i2 == 20);

    // alias null should create a weird ptr which is not null, but has a zero ref count
    int foo = 34;
    i = test::test_shared_ptr<int>(c, &foo);
    CHECK(i);
    CHECK(*i == 34);
    CHECK(i.use_count() == 0);
    auto i3 = i;
    CHECK(i3);
    CHECK(*i3 == 34);
    CHECK(i3.use_count() == 0);
    i.reset();
    CHECK_FALSE(i);

    i = xtest::make_aliased(c, &foo);
    CHECK_FALSE(i);
}

TEST_CASE("shared_ptr: self usurp") {
    obj::lifetime_stats stats;
    doctest::util::lifetime_counter_sentry _ls(stats);

    // self usurp
    {
        auto ptr = test::make_test_shared<obj>(10, "ten");
        auto& ref = ptr;
        ptr = ref;
        CHECK(ptr);
        CHECK(ptr.use_count() == 1);
        CHECK(ptr->a == 10);

        ptr = std::move(ref);
        CHECK(ptr);
        CHECK(ptr.use_count() == 1);
        CHECK(ptr->a == 10);

        ptr.swap(ref);
        CHECK(ptr);
        CHECK(ptr.use_count() == 1);
        CHECK(ptr->b == "ten");
    }

    // seme-cb
    {
        auto ptr = test::make_test_shared<obj>(10, "ten");

        auto cptr = ptr;

        ptr = cptr;
        CHECK(ptr.use_count() == 2);
        CHECK(cptr == ptr);

        cptr.swap(ptr);
        CHECK(ptr.use_count() == 2);
        CHECK(cptr == ptr);

        ptr = std::move(cptr);
        CHECK(ptr.use_count() == 1);

        CHECK(stats.living == 1);
    }

    // parent copy-over
    {
        auto pc = test::make_test_shared<child>(10, 20);
        test::test_shared_ptr<obj> po = pc;
        CHECK(pc.use_count() == 2);

        po = pc;
        CHECK(pc.use_count() == 2);

        po = std::move(pc);
        CHECK(po.use_count() == 1);

        CHECK(stats.living == 1);
    }

    // aliased copies
    {
        auto pc = test::make_test_shared<child>(10, 20);
        test::test_shared_ptr<int> pa(pc, &pc->a);
        test::test_shared_ptr<int> pb(pc, &pc->c);

        int a = 5, b = 6;
        test::test_shared_ptr<int> pi1(test::test_shared_ptr<void>{}, & a);
        test::test_shared_ptr<int> pi2(test::test_shared_ptr<void>{}, & b);

        CHECK(pc.use_count() == 3);

        pa.swap(pb);
        CHECK(pc.use_count() == 3);
        CHECK(pa != pb);
        CHECK(xtest::same_owner(pa, pb));
        CHECK(*pa == 20);
        CHECK(*pb == 10);

        pa = pb;
        CHECK(pc.use_count() == 3);
        CHECK(*pa == 10);

        pi1.swap(pi2);
        CHECK(pi1 != pi2);
        CHECK(*pi1 == 6);
        CHECK(*pi2 == 5);

        pa = pi1;
        CHECK(pc.use_count() == 2);

        pi1 = pb;
        CHECK(pc.use_count() == 3);

        CHECK(stats.living == 1);
    }

    CHECK(stats.total == 4);
}

TEST_CASE("shared_ptr: cast null") {
    test::test_shared_ptr<const multi_child> cptr;
    CHECK_FALSE(cptr);
    auto ptr = test::const_pointer_cast<multi_child>(cptr);
    CHECK_FALSE(ptr);
    test::test_shared_ptr<obj> as_obj = ptr;
    CHECK_FALSE(as_obj);
    test::test_shared_ptr<vec> as_vec = ptr;
    CHECK_FALSE(as_vec);
    auto sc = test::static_pointer_cast<multi_child>(as_vec);
    CHECK_FALSE(sc);
    auto dc = test::dynamic_pointer_cast<multi_child>(as_obj);
    CHECK_FALSE(dc);
    test::test_shared_ptr<void> erased = ptr;
    CHECK_FALSE(erased);
    auto rc = test::reinterpret_pointer_cast<multi_child>(erased);
    CHECK_FALSE(rc);
}

TEST_CASE("shared_ptr: cast") {
    obj::lifetime_stats stats;

    test::test_shared_ptr<const multi_child> cptr = test::make_test_shared<multi_child>();

    auto ptr = test::const_pointer_cast<multi_child>(cptr);
    CHECK(ptr.get() == cptr.get());
    CHECK(ptr.use_count() == 2);

    ptr->a = 42;
    ptr->x = 1;
    ptr->y = 2;

    test::test_shared_ptr<obj> as_obj = ptr;
    test::test_shared_ptr<vec> as_vec = ptr;

    CHECK(ptr.use_count() == 4);

    auto sc = test::static_pointer_cast<multi_child>(as_vec);
    CHECK(sc.get() == ptr.get());
    CHECK(ptr.use_count() == 5);

    auto dc = test::dynamic_pointer_cast<multi_child>(as_obj);
    CHECK(dc.get() == ptr.get());
    CHECK(ptr.use_count() == 6);

    auto fdc = test::dynamic_pointer_cast<child>(as_obj);
    CHECK_FALSE(fdc);
    CHECK(ptr.use_count() == 6);

    test::test_shared_ptr<void> erased = ptr;

    auto rc = test::reinterpret_pointer_cast<multi_child>(erased);
    CHECK(rc.get() == ptr.get());
    CHECK(ptr.use_count() == 8);

    CHECK(stats.total == 1);
    CHECK(stats.living == 1);
}