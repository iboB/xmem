// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <memory>

namespace xmem {

template <typename Alloc>
struct allocator_rebind;

template <template <typename> class Alloc, typename T>
struct allocator_rebind<Alloc<T>> {
    template <typename U>
    using to = Alloc<U>;
};

}
