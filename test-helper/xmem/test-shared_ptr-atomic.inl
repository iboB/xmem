// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//

// inline file - no include guard

#include <thread>
#include <mutex>

TEST_CASE("shared_ptr: mt") {
    // no sensible checks here
    // just confirm that there are no crashes and no sanitizer complaints

    obj::lifetime_stats stats;
    doctest::util::lifetime_counter_sentry _lcs(stats);

    std::mutex mto_a;
    std::vector<test::test_shared_ptr<obj>> to_a;
    std::mutex mto_b;
    std::vector<test::test_shared_ptr<obj>> to_b;
    std::mutex mto_c;
    std::vector<test::test_shared_ptr<obj>> to_c;

    std::atomic<bool> start{false};
    int sum_a = 0, sum_b = 0, sum_c = 0;


    std::thread t_a([&]() {
        while (!start);
        std::vector<test::test_shared_ptr<obj>> work;
        std::vector<test::test_shared_ptr<obj>> stored;
        int i = 0;
        while (true) {
            {
                std::lock_guard _l(mto_a);
                work.swap(to_a);
            }

            ++i;
            if (i < 10) {
                std::lock_guard _l(mto_c);
                to_c.push_back(test::make_test_shared<obj>(i));
            }
            if (i == 10) {
                std::lock_guard _l(mto_c);
                to_c.push_back({});
            }

            for (auto& p : work) {
                if (!p) {
                    if (i < 10) {
                        std::lock_guard _l(mto_c);
                        to_c.push_back({});
                    }
                    return;
                }
                if (p->val() == 10) {
                    stored.push_back(p);
                }
                else if (p->val() > 20) {
                    std::lock_guard _l(mto_b);
                    to_b.push_back(p);
                }

                sum_a += p->val();
            }
        }
    });

    std::thread t_b([&]() {
        while (!start);
        std::vector<test::test_shared_ptr<obj>> work;
        std::vector<test::test_shared_ptr<obj>> stored;
        int i = 0;

        while (true) {
            {
                std::lock_guard _l(mto_b);
                work.swap(to_b);
            }

            ++i;
            if (i < 10) {
                std::lock_guard _l(mto_a);
                to_a.push_back(test::make_test_shared<obj>(i * i));
            }
            if (i == 10) {
                std::lock_guard _l(mto_a);
                to_a.push_back({});
            }

            for (auto& p : work) {
                if (!p) {
                    if (i < 10) {
                        std::lock_guard _l(mto_a);
                        to_a.push_back({});
                    }
                    return;
                }
                sum_b += p->val();
                if (p->val() > 30) {
                    stored.push_back(p);
                }
            }
        }
    });

    std::thread t_c([&]() {
        while (!start);
        std::vector<test::test_shared_ptr<obj>> work;
        int i = 0;

        while (true) {
            {
                std::lock_guard _l(mto_c);
                work.swap(to_c);
            }

            ++i;
            if (i < 10) {
                std::lock_guard _l(mto_b);
                to_b.push_back(test::make_test_shared<child>(2 * i, 3 * i));
            }
            if (i == 10) {
                std::lock_guard _l(mto_b);
                to_b.push_back({});
            }

            for (auto& p : work) {
                if (!p) {
                    if (i < 10) {
                        std::lock_guard _l(mto_b);
                        to_b.push_back({});
                    }
                    return;
                }
                sum_c += p->val();
            }
        }
    });

    start = true;
    t_a.join();
    t_b.join();
    t_c.join();

    CHECK(sum_a > 0);
    CHECK(sum_b > 0);
    CHECK(sum_c > 0);
}

TEST_CASE("atomic_shared_ptr_storage: basic") {
    static_assert(sizeof(xtest::atomic_shared_ptr_storage<int>) <= 64, "We want true sharing here");

    {
        xtest::atomic_shared_ptr_storage<int> pi;
        CHECK_FALSE(pi.load());
        pi.store({});
        CHECK_FALSE(pi.load());

        pi.store(test::make_test_shared<int>(15));
        {
            auto p = pi.load();
            CHECK(p);
            CHECK(p.use_count() == 2);
            CHECK(*p == 15);
        }

        pi.store({});
        CHECK_FALSE(pi.load());
    }

    {
        auto ptr1 = test::make_test_shared<int>(11);
        xtest::atomic_shared_ptr_storage<int> pi(ptr1);
        auto ptr2 = test::make_test_shared<int>(32);
        auto ret = pi.exchange(ptr2);
        CHECK(ret == ptr1);
        CHECK(pi.load() == ptr2);

        auto ptr3 = test::make_test_shared<int>(99);
        CHECK_FALSE(pi.compare_exchange(ptr1, ptr3));
        CHECK(ptr1 == ptr2);
        CHECK(pi.load() == ptr2);
        CHECK(pi.compare_exchange(ptr2, ptr3));
        CHECK(ptr1 == ptr2);
        CHECK(pi.load() == ptr3);

        CHECK(pi.compare_exchange(ptr3, {}));
        CHECK_FALSE(pi.load());

        ret = pi.exchange({});
        CHECK_FALSE(ret);

        test::test_shared_ptr<int> empty;
        CHECK(pi.compare_exchange(empty, ptr1));
        CHECK_FALSE(empty);
        CHECK(pi.load() == ptr1);

        ret = pi.exchange(ptr1);
        CHECK(ret == ptr1);
        CHECK(ptr1 == ptr2);
        CHECK(ptr1.use_count() == 4);

        CHECK(ptr3.use_count() == 1);
    }
}

TEST_CASE("atomic_shared_ptr_storage: load/store") {
    // no sensible checks here
    // just confirm that there are no crashes and no sanitizer complaints

    std::atomic<bool> start{false};
    std::atomic<int> sum{0};

    xtest::atomic_shared_ptr_storage<int> storage(test::make_test_shared<int>(10000));

    std::thread a([&]() {
        while (!start);
        for (int i = 0; i < 50; ++i) {
            sum += *storage.load();
            storage.store(test::make_test_shared<int>(i));
        }
    });

    std::thread b([&]() {
        while (!start);
        for (int i = 0; i < 50; ++i) {
            sum += *storage.load();
            storage.store(test::make_test_shared<int>(i * 10));
        }
    });

    start = true;
    a.join();
    b.join();

    CHECK(storage.load().use_count() == 2);
    CHECK(sum > 10000);
}

TEST_CASE("atomic_shared_ptr_storage: exchange") {
    // no sensible checks here as well
    // just confirm that there are no crashes and no sanitizer complaints

    std::atomic<bool> start{false};
    std::atomic<int> sum{0};

    auto init = test::make_test_shared<int>(-1);
    xtest::atomic_shared_ptr_storage<int> storage(init);

    auto a = test::make_test_shared<int>(1);
    auto b = test::make_test_shared<int>(2);

    std::thread ta([&]() {
        while (!start);
        for (int i = 0; i < 50; ++i) {
            auto ret = storage.exchange(a);
            if (ret != a) ++sum;
        }
    });

    std::thread tb([&]() {
        while (!start);
        for (int i = 0; i < 50; ++i) {
            auto ac = a;
            auto success = storage.compare_exchange(ac, b);
            if (!success) {
                CHECK((ac == b || ac == init));
            }
            else {
                ++sum;
                CHECK(ac == a);
            }
        }
    });

    start = true;
    ta.join();
    tb.join();

    // if thread b completed all of its iterations before a managed to do one,
    // we will end up with 100% fail rate in b and a stored in storage
    {
        auto ac = a;
        sum += storage.compare_exchange(ac, b);
    }

    CHECK(sum >= 2);

    storage.exchange({});
    CHECK_FALSE(storage.load());

    CHECK(a.use_count() == 1);
    CHECK(b.use_count() == 1);
}
