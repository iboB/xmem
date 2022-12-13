// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//

// inline file - no include guard

// weak_ptr and shared_from basic test

TEST_CASE("weak_ptr basic") {
    using sptr = test::test_shared_ptr<obj>;
    using wptr = test::test_weak_ptr<obj>;

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
        auto se = test::make_test_shared<child>(5, 10);
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
        auto sp = test::make_test_shared<obj>(5, "xyz");
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

#if ENABLE_XMEM_SPECIFIC_CHECKS
TEST_CASE("weak_ptr: alias") {
    auto c = test::make_test_shared<child>(10, 20);

    test::test_weak_ptr<int> wca(c, &c->a);
    CHECK_FALSE(wca.expired());
    XMEM(CHECK(wca.owner() == c.owner()));

    test::test_weak_ptr<int> wcc(wca, &c->c);
    CHECK_FALSE(wcc.expired());
    XMEM(CHECK(wcc.owner() == wca.owner()));

    CHECK(wca.use_count() == 1);

    CHECK(wca.lock().get() == &c->a);
    CHECK(wcc.lock().get() == &c->c);

    c.reset();

    CHECK(wca.expired());
    CHECK(wcc.expired());

    // alias null should be safe
    wca = test::test_weak_ptr<int>(c, &c->a);
    CHECK_FALSE(wca);
    CHECK(wca.expired());

    wcc = test::test_weak_ptr<int>(wca, &c->c);
    CHECK_FALSE(wcc);
    CHECK(wcc.expired());
}
#endif

//////////////////////////////////////////////////////////

#if ENABLE_XMEM_SPECIFIC_CHECKS

class sf_type : public test::enable_test_shared_from {
public:
    int id = 0;

    using test::enable_test_shared_from::weak_from_this;
    using test::enable_test_shared_from::shared_from_this;

    test::test_shared_ptr<sf_type> clone() {
        return shared_from(this);
    }
};

TEST_CASE("shared_from: basic") {
    {
        sf_type t;
        auto sp = t.shared_from_this();
        CHECK_FALSE(sp);
        CHECK(sp.use_count() == 0);
        auto wp = t.weak_from_this();
        CHECK_FALSE(wp);
        CHECK_FALSE(wp.owner());
        CHECK_FALSE(t.clone());
    }

    {
        auto sfptr = test::make_test_shared<sf_type>();
        sfptr->id = 3;
        {
            auto sp = sfptr->shared_from_this();
            CHECK(sp == sfptr);
            CHECK(sp.owner() == sfptr.owner());
            CHECK(sp.use_count() == 2);
        }
        {
            auto cpy = *sfptr;
            CHECK(cpy.id == 3);
            CHECK_FALSE(cpy.shared_from_this());
            CHECK_FALSE(cpy.weak_from_this());
        }
        {
            sf_type cpy;
            CHECK(cpy.id == 0);
            CHECK_FALSE(cpy.shared_from_this());
            CHECK_FALSE(cpy.weak_from_this());
            cpy = *sfptr;
            CHECK(cpy.id == 3);
            CHECK_FALSE(cpy.shared_from_this());
            CHECK_FALSE(cpy.weak_from_this());
        }
        {
            auto mve = std::move(*sfptr);
            CHECK(mve.id == 3);
            CHECK_FALSE(mve.shared_from_this());
            CHECK_FALSE(mve.weak_from_this());
            CHECK(sfptr->shared_from_this());
            CHECK(sfptr->weak_from_this());
        }
        {
            sf_type mve;
            CHECK(mve.id == 0);
            CHECK_FALSE(mve.shared_from_this());
            CHECK_FALSE(mve.weak_from_this());
            mve = std::move(*sfptr);
            CHECK(mve.id == 3);
            CHECK_FALSE(mve.shared_from_this());
            CHECK_FALSE(mve.weak_from_this());
        }

        auto wp = sfptr->weak_from_this();
        CHECK(wp);
        CHECK(wp.owner() == sfptr.owner());
        CHECK(wp.use_count() == 1);
        CHECK(wp.lock() == sfptr->clone());

        sfptr.reset();
        CHECK(wp.expired());
    }
}

#endif

class sft_type : public test::enable_test_shared_from_this<sft_type> {
public:
    int id = 0;
};

TEST_CASE("shared_from_this: basic") {
    auto ptr = test::make_test_shared<sft_type>();
    ptr->id = 3;

    {
        auto clone = ptr->shared_from_this();
        CHECK(clone->id == 3);
        CHECK(clone == ptr);
        CHECK(clone.use_count() == 2);

        auto wc = ptr->weak_from_this();
        XMEM(CHECK(wc));
        XMEM(CHECK(wc.owner() == ptr.owner()));
        CHECK(wc.use_count() == 2);
        CHECK(wc.lock()->id == 3);
    }

    test::test_shared_ptr<const sft_type> cptr = ptr;
}