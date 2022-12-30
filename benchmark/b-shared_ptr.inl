// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <picobench/picobench.hpp>
#include <random>

// inline file - no include guard

struct integer {
    integer() = default;
    integer(uint32_t n) : val(n) {}
    uint32_t val;
};

void FUNC(picobench::state& pb) {
    std::minstd_rand rnd(42);
    auto size = size_t(pb.iterations());
    std::vector<sptr<integer>> salloc;
    salloc.reserve(size * 2);
    std::vector<wptr<integer>> walloc(size);

    uint32_t sum = 0;
    picobench::scope scope(pb);

    auto shared = std::move(salloc);
    auto weak = std::move(walloc);

    for (auto& w : weak) {
        auto ptr = make<integer>(rnd());
        w = ptr;
        shared.push_back(std::move(ptr));
    }
    // duplicate
    for (size_t i = 0; i < size; ++i) {
        shared.push_back(shared[i]);
    }
    // have roughly one fourth of shared expire
    for (auto& s : shared) {
        if (rnd() % 2) {
            s.reset();
        }
    }
    // sum non-expired weak
    for (auto& w : weak) {
        auto s = w.lock();
        if (!s) continue;
        sum += s->val;
    }

    pb.set_result(sum);
}
