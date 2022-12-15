// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//

// this example demonstrates the catching of a shared pointer leak

#include <iostream>

#include <xmem/atomic_ref_count.hpp>
#include <xmem/common_control_block.hpp>

#include <mutex>
#include <unordered_set>
#include <algorithm>
#include <vector>

#define B_STACKTRACE_IMPL
#include <b_stacktrace.h>

namespace myapp {

struct bookkeeping_control_block;
namespace bookkeeping_control_block_registry {
    std::mutex m_mutex;
    std::unordered_set<const bookkeeping_control_block*> m_blocks;

    void add(const bookkeeping_control_block* block) {
        std::lock_guard _l(m_mutex);
        m_blocks.insert(block);
    }
    void remove(const bookkeeping_control_block* block) {
        std::lock_guard _l(m_mutex);
        auto f = m_blocks.find(block);
        m_blocks.erase(f);
    }
    std::vector<const bookkeeping_control_block*> get() {
        std::lock_guard _l(m_mutex);
        return {m_blocks.begin(), m_blocks.end()};
    }
};

struct bookkeeping_control_block : protected xmem::control_block_base<xmem::atomic_ref_count> {
    bookkeeping_control_block() {
        bookkeeping_control_block_registry::add(this);
    }
    ~bookkeeping_control_block() {
        bookkeeping_control_block_registry::remove(this);
    }

    using super = xmem::control_block_base<xmem::atomic_ref_count>;

    std::mutex m_mutex;

    struct entry {
        struct free_deleter {
            void operator()(char* ptr) { free(ptr); }
        };

        const void* ptr;
        xmem::unique_ptr<char, free_deleter> stacktrace;
    };

    std::vector<entry> active_strong;

    void on_new_strong(const void* src) {
        std::lock_guard _l(m_mutex);
        auto& e = active_strong.emplace_back();
        e.ptr = src;
        e.stacktrace.reset(b_stacktrace_get());
    }

    void on_destroy_strong(const void* src) {
        std::lock_guard _l(m_mutex);
        auto f = std::find_if(active_strong.begin(), active_strong.end(), [&](const entry& e) { return e.ptr == src; });
        active_strong.erase(f);
    }

    void on_new_weak(const void* /*src*/) {
        // std::lock_guard _l(m_mutex);
    }

    void on_destroy_weak(const void* /*src*/) {
        // std::lock_guard _l(m_mutex);
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

using bookkeeping_control_block_factory = xmem::control_block_factory<bookkeeping_control_block>;

template <typename T>
using shared_ptr = xmem::basic_shared_ptr<bookkeeping_control_block_factory, T>;

template <typename T>
using weak_ptr = xmem::basic_weak_ptr<bookkeeping_control_block_factory, T>;

using shared_from = xmem::basic_enable_shared_from<bookkeeping_control_block_factory>;

template <typename T, typename... Args>
[[nodiscard]] shared_ptr<T> make_shared(Args&&... args) {
    return shared_ptr<T>(bookkeeping_control_block_factory::make_resource_cb<T>(std::allocator<char>{}, std::forward<Args>(args)...));
}

}

void foo(myapp::shared_ptr<const void> pl) {
    auto blocks = myapp::bookkeeping_control_block_registry::get();
    for (auto b : blocks) {
        std::cout << "============= " << b << " ===============\n\n";
        for (auto& r : b->active_strong) {
            std::cout << r.stacktrace.get() << "\n\n";
        }
    }
}

int main() {
    auto p = myapp::make_shared<int>(34);
    auto p2 = myapp::make_shared<int>(22);
    foo(std::move(p2));
}