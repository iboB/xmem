// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//

// inline file - no include guard

struct integer {
    integer() = default;
    integer(uint32_t n) : val(n) {}
    uint32_t val;
};

void uptr(picobench::state& pb) {
    std::minstd_rand rnd(32);
    std::vector<unique_ptr<integer>> alloc(size_t(pb.iterations())); // keep initial allocation out of scope
    uint32_t sum = 0;

    picobench::scope scope(pb);
    auto ints = std::move(alloc);
    for (auto& i : ints) {
        if (rnd() % 3) i = make_unique<integer>(rnd());
    }
    for (auto& i : ints) {
        if (i) sum += i->val;
    }

    pb.set_result(sum);
}
