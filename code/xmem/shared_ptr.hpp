// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "common_control_block.hpp"
#include "atomic_ref_count.hpp"
#include "basic_atomic_shared_ptr_storage.hpp"

namespace xmem {

using atomic_control_block_factory = control_block_factory<control_block_base<atomic_ref_count>>;

template <typename T>
using shared_ptr = basic_shared_ptr<atomic_control_block_factory, T>;

template <typename T>
using weak_ptr = basic_weak_ptr<atomic_control_block_factory, T>;

using enable_shared_from = basic_enable_shared_from<atomic_control_block_factory>;

template <typename T>
using enable_shared_from_this = basic_enable_shared_from_this<atomic_control_block_factory, T>;

template <typename T, typename... Args>
[[nodiscard]] shared_ptr<T> make_shared(Args&&... args) {
    return shared_ptr<T>(atomic_control_block_factory::make_resource_cb<T>(allocator<char>{}, std::forward<Args>(args)...));
}

template <typename T>
[[nodiscard]] auto make_shared_ptr(T&& t) -> shared_ptr<std::remove_reference_t<T>> {
    // weirdly enough msvc doesn't like this make_shared call without the explicit xmem::
    return xmem::make_shared<std::remove_reference_t<T>>(std::forward<T>(t));
}

template <typename T>
[[nodiscard]] shared_ptr<T> make_shared_for_overwrite() {
    return shared_ptr<T>(atomic_control_block_factory::make_resource_cb_for_overwrite<T>(allocator<char>{}));
}

template <typename T>
using atomic_shared_ptr_storage = basic_atomic_shared_ptr_storage<atomic_control_block_factory, T>;

}