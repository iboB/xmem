// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "basic_shared_ptr.hpp"
#include "basic_weak_ptr.hpp"
#include "local_control_block.hpp"

namespace xmem {
template <typename T>
using local_shared_ptr = basic_shared_ptr<T, local_control_block>;
template <typename T>
using local_weak_ptr = basic_weak_ptr<T, local_control_block>;
}