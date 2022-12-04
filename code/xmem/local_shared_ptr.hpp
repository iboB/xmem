// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "basic_shared_ptr.hpp"
#include "basic_weak_ptr.hpp"
#include "local_control_block.hpp"

namespace xmem {
template <typename T>
using local_shared_ptr = basic_shared_ptr<local_control_block, T>;
template <typename T>
using local_weak_ptr = basic_weak_ptr<local_control_block, T>;

template <typename T, typename... Args>
[[nodiscard]] local_shared_ptr<T> make_local_shared(Args&&... args) {
    return local_shared_ptr<T>(local_control_block::make_resource_cb<T>(std::forward<Args>(args)...));
}

template <typename T>
[[nodiscard]] auto make_local_shared_ptr(T&& t) -> local_shared_ptr<std::remove_reference_t<T>> {
    return make_local_shared<std::remove_reference_t<T>>(std::forward<T>(t));
}

template <typename T>
[[nodiscard]] local_shared_ptr<T> make_local_shared_for_overwrite() {
    return local_shared_ptr<T>(local_control_block::make_resource_cb_for_overwrite<T>());
}

}
