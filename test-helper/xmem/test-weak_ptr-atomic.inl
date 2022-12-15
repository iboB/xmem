// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//

// inline file - no include guard

TEST_CASE("weak_ptr: atomic") {
    // no sensible checks here
    // just confirm that there are no crashes and no sanitizer complaints

    obj::lifetime_stats stats;
    doctest::util::lifetime_counter_sentry _lcs(stats);

    std::atomic<bool> start{false};

    std::vector<test::test_shared_ptr<obj>> objects;
    std::vector<test::test_weak_ptr<obj>> weaks;
    for (int i = 0; i < 100; ++i) {
        auto o = test::make_test_shared<child>(i, 10 * i);
        objects.push_back(o);
        weaks.push_back(o);
    }

    test::test_shared_ptr<obj> perm = test::make_test_shared<obj>(1, "perm");
    weaks.push_back(perm);

    std::thread producer([&start, &objects]() {
        while (!start);

        while (!objects.empty()) {
            int i = rand() % int(2 * objects.size());
            if (i >= int(objects.size())) continue;
            objects.erase(objects.begin() + i);
        }
    });

    long long sum = 0;
    std::thread consumer([&start, &weaks, &sum]() {
        while (!start);

        while (true) {
            int living = 0;
            for (auto& w : weaks) {
                if (auto o = w.lock()) {
                    ++living;
                    sum += o->val();
                }
            }
            if (living == 1) {
                break;
            }
        }
    });

    start = true;
    producer.join();
    consumer.join();

    CHECK(sum > 0);
    CHECK(stats.living == 1);
    CHECK(stats.total == weaks.size());
}