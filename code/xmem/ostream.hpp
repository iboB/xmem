// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "unique_ptr.hpp"
#include "basic_shared_ptr.hpp"

#include <ostream>

namespace xmem {
template <typename T, typename D>
std::ostream& operator<<(std::ostream& out, const unique_ptr<T, D>& ptr) {
    out << ptr.get();
    return out;
}

template <typename CBF, typename T>
std::ostream& operator<<(std::ostream& out, const basic_shared_ptr<CBF, T>& ptr) {
    out << ptr.get();
    return out;
}
}
