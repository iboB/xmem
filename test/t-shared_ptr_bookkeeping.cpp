// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#define XMEM_TEST_NAMESPACE xmem
#define ENABLE_XMEM_SPECIFIC_CHECKS 1
#include <xmem/test_init.inl>

#include <xmem/local_ref_count.hpp>
#include <xmem/common_control_block.hpp>

#include <unordered_set>

#include <doctest/doctest.h>
TEST_SUITE_BEGIN("shared_ptr bookkeeping");

namespace xmem {

struct bookkeeping_control_block : protected control_block_base<local_ref_count> {
    using super = control_block_base<local_ref_count>;

    std::unordered_set<const void*> active_strong;
    std::unordered_set<const void*> active_weak;

    ~bookkeeping_control_block() {
        CHECK(active_strong.empty());
        CHECK(active_weak.empty());
    }

    void on_new_strong(const void* src) {
        REQUIRE(active_strong.find(src) == active_strong.end());
        active_strong.insert(src);
    }

    void on_destroy_strong(const void* src) {
        auto f = active_strong.find(src);
        REQUIRE(f != active_strong.end());
        active_strong.erase(f);
    }

    void on_new_weak(const void* src) {
        REQUIRE(active_weak.find(src) == active_weak.end());
        active_weak.insert(src);
    }

    void on_destroy_weak(const void* src) {
        auto f = active_weak.find(src);
        REQUIRE(f != active_weak.end());
        active_weak.erase(f);
    }

    void init_strong(const void* src) noexcept {
        super::init_strong(src);
        on_new_strong(src);
    }

    void inc_strong_ref(const void* src) noexcept {
        super::inc_strong_ref(src);
        on_new_strong(src);
    }
    void dec_strong_ref(const void* src) noexcept {
        on_destroy_strong(src);
        super::dec_strong_ref(src);
    }
    bool inc_strong_ref_nz(const void* src) noexcept {
        if (super::inc_strong_ref_nz(src)) {
            on_new_strong(src);
            return true;
        }
        return false;
    }
    using super::strong_ref_count;

    void transfer_strong(const void* dest, const void* src) {
        super::transfer_strong(dest, src);
        on_new_strong(dest);
        on_destroy_strong(src);
    }

    void inc_weak_ref(const void* src) noexcept {
        super::inc_weak_ref(src);
        on_new_weak(src);
    }
    void dec_weak_ref(const void* src) noexcept {
        on_destroy_weak(src);
        super::dec_weak_ref(src);
    }
    void transfer_weak(const void* dest, const void* src) {
        super::transfer_weak(dest, src);
        on_new_weak(dest);
        on_destroy_weak(src);
    }
};

using bookkeeping_control_block_factory = control_block_factory<bookkeeping_control_block>;

template <typename T>
using bookkeeping_shared_ptr = basic_shared_ptr<bookkeeping_control_block_factory, T>;

template <typename T>
using bookkeeping_weak_ptr = basic_weak_ptr<bookkeeping_control_block_factory, T>;

using enable_bookkeeping_shared_from = basic_enable_shared_from<bookkeeping_control_block_factory>;

template <typename T>
using enable_bookkeeping_shared_from_this = basic_enable_shared_from_this<bookkeeping_control_block_factory, T>;

template <typename T, typename... Args>
[[nodiscard]] bookkeeping_shared_ptr<T> make_bookkeeping_shared(Args&&... args) {
    return bookkeeping_shared_ptr<T>(bookkeeping_control_block_factory::make_resource_cb<T>(allocator<char>{}, std::forward<Args>(args)...));
}

template <typename T>
[[nodiscard]] auto make_bookkeeping_shared_ptr(T&& t) -> bookkeeping_shared_ptr<std::remove_reference_t<T>> {
    return make_bookkeeping_shared<std::remove_reference_t<T>>(std::forward<T>(t));
}

template <typename T>
[[nodiscard]] bookkeeping_shared_ptr<T> make_bookkeeping_shared_for_overwrite() {
    return bookkeeping_shared_ptr<T>(bookkeeping_control_block_factory::make_resource_cb_for_overwrite<T>(allocator<char>{}));
}

}

#define test_shared_ptr bookkeeping_shared_ptr
#define make_test_shared make_bookkeeping_shared
#define make_test_shared_ptr make_bookkeeping_shared_ptr
#define make_test_shared_for_overwrite make_bookkeeping_shared_for_overwrite

#include <xmem/test-shared_ptr-local.inl>

#define test_weak_ptr bookkeeping_weak_ptr
#define enable_test_shared_from enable_bookkeeping_shared_from
#define enable_test_shared_from_this enable_bookkeeping_shared_from_this

#include <xmem/test-weak_ptr-shared_from-local.inl>
