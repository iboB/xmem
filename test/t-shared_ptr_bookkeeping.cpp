// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <doctest/doctest.h>

#include <xmem/local_ref_count.hpp>
#include <xmem/common_control_block.hpp>

#include <unordered_set>

TEST_SUITE_BEGIN("shared_ptr bookkeeping");

namespace bktest {

struct bookkeeping_control_block : protected xmem::control_block_base<xmem::local_ref_count> {
    using super = xmem::control_block_base<xmem::local_ref_count>;

    std::unordered_set<const void*> active_strong;
    std::unordered_set<const void*> active_weak;

    ~bookkeeping_control_block() {
        CHECK(active_strong.empty());
        CHECK(active_weak.empty());
    }

    void on_new_strong(const void* src) {
        REQUIRE(active_strong.find(src) == active_strong.end());
        active_strong.insert(src);
    }

    void on_destroy_strong(const void* src) {
        auto f = active_strong.find(src);
        REQUIRE(f != active_strong.end());
        active_strong.erase(f);
    }

    void on_new_weak(const void* src) {
        REQUIRE(active_weak.find(src) == active_weak.end());
        active_weak.insert(src);
    }

    void on_destroy_weak(const void* src) {
        auto f = active_weak.find(src);
        REQUIRE(f != active_weak.end());
        active_weak.erase(f);
    }

    void init_strong(const void* src) noexcept {
        super::init_strong(src);
        on_new_strong(src);
    }

    void inc_strong_ref(const void* src) noexcept {
        super::inc_strong_ref(src);
        on_new_strong(src);
    }
    void dec_strong_ref(const void* src) noexcept {
        on_destroy_strong(src);
        super::dec_strong_ref(src);
    }
    bool inc_strong_ref_nz(const void* src) noexcept {
        if (super::inc_strong_ref_nz(src)) {
            on_new_strong(src);
            return true;
        }
        return false;
    }
    using super::strong_ref_count;

    void transfer_strong(const void* dest, const void* src) {
        super::transfer_strong(dest, src);
        on_new_strong(dest);
        on_destroy_strong(src);
    }

    void inc_weak_ref(const void* src) noexcept {
        super::inc_weak_ref(src);
        on_new_weak(src);
    }
    void dec_weak_ref(const void* src) noexcept {
        on_destroy_weak(src);
        super::dec_weak_ref(src);
    }
    void transfer_weak(const void* dest, const void* src) {
        super::transfer_weak(dest, src);
        on_new_weak(dest);
        on_destroy_weak(src);
    }
};

using bookkeeping_control_block_factory = xmem::control_block_factory<bookkeeping_control_block>;

template <typename T>
using bookkeeping_shared_ptr = xmem::basic_shared_ptr<bookkeeping_control_block_factory, T>;

template <typename T>
using bookkeeping_weak_ptr = xmem::basic_weak_ptr<bookkeeping_control_block_factory, T>;

using enable_bookkeeping_shared_from = xmem::basic_enable_shared_from<bookkeeping_control_block_factory>;

template <typename T>
using enable_bookkeeping_shared_from_this = xmem::basic_enable_shared_from_this<bookkeeping_control_block_factory, T>;

template <typename T, typename... Args>
[[nodiscard]] bookkeeping_shared_ptr<T> make_bookkeeping_shared(Args&&... args) {
    return bookkeeping_shared_ptr<T>(bookkeeping_control_block_factory::make_resource_cb<T>(xmem::allocator<char>{}, std::forward<Args>(args)...));
}

template <typename T>
[[nodiscard]] auto make_bookkeeping_shared_ptr(T&& t) -> bookkeeping_shared_ptr<std::remove_reference_t<T>> {
    return make_bookkeeping_shared<std::remove_reference_t<T>>(std::forward<T>(t));
}

template <typename T>
[[nodiscard]] bookkeeping_shared_ptr<T> make_bookkeeping_shared_for_overwrite() {
    return bookkeeping_shared_ptr<T>(bookkeeping_control_block_factory::make_resource_cb_for_overwrite<T>(xmem::allocator<char>{}));
}

}

#define XMEM_TEST_NAMESPACE bktest
#define ENABLE_XMEM_SPECIFIC_CHECKS 1
#include <xmem/test_init.inl>

TEST_CASE("shared_ptr: basic") {
    using sptr = bktest::bookkeeping_shared_ptr<obj>;

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

    using siptr = bktest::bookkeeping_shared_ptr<int>;
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
        auto sptr = bktest::make_bookkeeping_shared<obj>();
        CHECK(sptr->a == 11);
        CHECK(sptr->b.empty());
    }

    {
        auto sptr = bktest::make_bookkeeping_shared<obj>(1, "abc");
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
    auto ip = bktest::make_bookkeeping_shared_for_overwrite<int>();
    CHECK(ip);

    auto op = bktest::make_bookkeeping_shared_for_overwrite<obj>();
    CHECK(op->a == 11);
})

XMEM(TEST_CASE("make_shared_ptr") {
    std::vector<int> vec = {1, 2, 3};
    auto copy = bktest::make_bookkeeping_shared_ptr(vec);
    CHECK(copy->size() == 3);
    CHECK(vec.size() == 3);
    CHECK(vec.data() != copy->data());
    copy->at(1) = 5;
    CHECK(*copy == std::vector<int>({1, 5, 3}));
    auto vdata = vec.data();
    auto heist = bktest::make_bookkeeping_shared_ptr(std::move(vec));
    CHECK(heist->size() == 3);
    CHECK(heist->data() == vdata);
})

TEST_CASE("shared_ptr: cast and type erasure") {
    obj::lifetime_stats stats;

    auto c = bktest::make_bookkeeping_shared<child>(1, 2);

    bktest::bookkeeping_shared_ptr<obj> p = c;
    CHECK(p);
    CHECK(p.use_count() == 2);
    CHECK(p->val() == 3);

    {
        bktest::bookkeeping_shared_ptr<const obj> cp = c;
        CHECK(cp);
        CHECK(cp.use_count() == 3);
        CHECK(cp->val() == 3);
    }

    bktest::bookkeeping_shared_ptr<void> v = p;
    p.reset();
    c.reset();

    CHECK(v.use_count() == 1);

    auto rp = reinterpret_cast<obj*>(v.get());
    CHECK(rp->val() == 3);
}

TEST_CASE("shared_ptr: alias") {
    obj::lifetime_stats stats;

    auto c = bktest::make_bookkeeping_shared<child>(10, 20);

    auto i = bktest::bookkeeping_shared_ptr<int>(c, &c->a);
    CHECK(i.get() == &c->a);
    XMEM(CHECK(i.owner() == c.owner()));

    auto i2 = bktest::bookkeeping_shared_ptr<int>(c, &c->c);
    CHECK(i2.get() == &c->c);
    XMEM(CHECK(i2.owner() == i.owner()));

    CHECK(i.use_count() == 3);

    c.reset();

    CHECK(i2.use_count() == 2);

    CHECK(*i == 10);
    CHECK(*i2 == 20);

    // alias null should be safe
    i = bktest::bookkeeping_shared_ptr<int>(c, &c->a);
    CHECK_FALSE(i);
    CHECK(i.use_count() == 0);
}

//////////////////////////////////////////////////////////

TEST_CASE("weak_ptr basic") {
    using sptr = bktest::bookkeeping_shared_ptr<obj>;
    using wptr = bktest::bookkeeping_weak_ptr<obj>;

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
        auto se = bktest::make_bookkeeping_shared<child>(5, 10);
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
        auto sp = bktest::make_bookkeeping_shared<obj>(5, "xyz");
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

TEST_CASE("weak_ptr: alias") {
    auto c = bktest::make_bookkeeping_shared<child>(10, 20);

    bktest::bookkeeping_weak_ptr<int> wca(c, &c->a);
    CHECK_FALSE(wca.expired());
    XMEM(CHECK(wca.owner() == c.owner()));

    bktest::bookkeeping_weak_ptr<int> wcc(wca, &c->c);
    CHECK_FALSE(wcc.expired());
    XMEM(CHECK(wcc.owner() == wca.owner()));

    CHECK(wca.use_count() == 1);

    CHECK(wca.lock().get() == &c->a);
    CHECK(wcc.lock().get() == &c->c);

    c.reset();

    CHECK(wca.expired());
    CHECK(wcc.expired());

    // alias null should be safe
    wca = bktest::bookkeeping_weak_ptr<int>(c, &c->a);
    CHECK_FALSE(wca);
    CHECK(wca.expired());

    wcc = bktest::bookkeeping_weak_ptr<int>(wca, &c->c);
    CHECK_FALSE(wcc);
    CHECK(wcc.expired());
}

//////////////////////////////////////////////////////////

class sf_type : public bktest::enable_bookkeeping_shared_from {
public:
    int id = 0;

    using bktest::enable_bookkeeping_shared_from::weak_from_this;
    using bktest::enable_bookkeeping_shared_from::shared_from_this;

    bktest::bookkeeping_shared_ptr<sf_type> clone() {
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
        auto sfptr = bktest::make_bookkeeping_shared<sf_type>();
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

class sft_type : public bktest::enable_bookkeeping_shared_from_this<sft_type> {
public:
    int id = 0;
};

TEST_CASE("shared_from_this: basic") {
    auto ptr = bktest::make_bookkeeping_shared<sft_type>();
    ptr->id = 3;

    {
        auto clone = ptr->shared_from_this();
        CHECK(clone->id == 3);
        CHECK(clone == ptr);
        CHECK(clone.use_count() == 2);

        auto wc = ptr->weak_from_this();
        CHECK(wc);
        CHECK(wc.owner() == ptr.owner());
        CHECK(wc.use_count() == 2);
        CHECK(wc.lock()->id == 3);
    }

    bktest::bookkeeping_shared_ptr<const sft_type> cptr = ptr;
}