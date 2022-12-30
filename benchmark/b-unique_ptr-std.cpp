// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <picobench/picobench.hpp>
#include <memory>
#include <random>

namespace std {
#include "b-unique_ptr.inl"
}

PICOBENCH(std::uptr);
