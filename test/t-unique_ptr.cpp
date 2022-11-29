// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <doctest/doctest.h>

#include <xmem/unique_ptr.hpp>

#include <vector>

TEST_SUITE_BEGIN("unique_ptr");

TEST_CASE("make_unique_ptr")
{
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
