// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//

// this example demonstrates the catching of a shared pointer leak

#include <iostream>
#include <vector>
#include <ctime>

#define TRACK_SHARED_PTR_LEAKS 1

#if TRACK_SHARED_PTR_LEAKS

#include <mutex>
#include <unordered_set>
#include <algorithm>
#include <iomanip>

#include <xmem/atomic_ref_count.hpp>
#include <xmem/common_control_block.hpp>
#include <xmem/ostream.hpp>

#define B_STACKTRACE_IMPL
#include <b_stacktrace.h>

namespace myapp {

struct free_deleter {
    void operator()(void* ptr) { free(ptr); }
};

struct stacktrace {
    static constexpr bool init = true;
    stacktrace(bool init = false) {
        if (init) record();
    }
    xmem::unique_ptr<b_stacktrace_tag, free_deleter> m_trace;
    explicit operator bool() { return !!m_trace; }
    void record() {
        m_trace.reset(b_stacktrace_get());
    }
    friend std::ostream& operator<<(std::ostream& out, const stacktrace& st) {
        if (st.m_trace) {
            auto str = xmem::unique_ptr<char, free_deleter>(b_stacktrace_to_string(st.m_trace.get()));
            out << str.get();
        }
        else {
            out << "<no stacktrace available>";
        }
        return out;
    }
};

class bookkeeping_control_block : protected xmem::control_block_base<xmem::atomic_ref_count> {
public:
    struct entry {
        const void* ptr;
        stacktrace trace;
    };
private:
    std::time_t m_creation_time;
    stacktrace m_creation_trace;
    mutable std::mutex m_mutex;
    std::vector<entry> m_active_strong;
public:

    bookkeeping_control_block()
        : m_creation_time(std::time(nullptr))
        , m_creation_trace(stacktrace::init)
    {}

    const time_t& creation_time() const { return m_creation_time; }
    const stacktrace& creation_trace() const { return m_creation_trace; }
    void log_active_strong(std::ostream& out) const {
        std::lock_guard _l(m_mutex);
        for (auto& ref : m_active_strong) {
            out << ref.ptr << ":\n";
            out << ref.trace << "\n";
        }
    }

    using super = xmem::control_block_base<xmem::atomic_ref_count>;

    void on_new_strong(const void* src) {
        std::lock_guard _l(m_mutex);
        auto& e = m_active_strong.emplace_back();
        e.ptr = src;
        e.trace.record();
    }

    void on_destroy_strong(const void* src) {
        std::lock_guard _l(m_mutex);
        auto f = std::find_if(m_active_strong.begin(), m_active_strong.end(), [&](const entry& e) { return e.ptr == src; });
        m_active_strong.erase(f);
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

template <typename T, typename... Args>
[[nodiscard]] shared_ptr<T> make_shared(Args&&... args) {
    return shared_ptr<T>(bookkeeping_control_block_factory::make_resource_cb<T>(std::allocator<char>{}, std::forward<Args>(args)...));
}

}

#else
#include <memory>
namespace myapp {
using std::shared_ptr;
using std::weak_ptr;
using std::make_shared;
}
#endif

using session = int;

auto session_factory(int id) {
    return myapp::make_shared<session>(id);
}

int main() {
    myapp::shared_ptr<session> leak;

    std::vector<myapp::weak_ptr<session>> registry;

    constexpr int N = 20;
    srand(unsigned(std::time(nullptr)));
    auto i_to_leak = rand() % (2 * N);
    for (int i = 0; i < N; ++i) {
        auto sptr = session_factory(i);
        registry.push_back(sptr);
        if (i == i_to_leak) {
            leak = sptr;
        }
    }

    for (auto& w : registry) {
        if (w.use_count()) {
#if TRACK_SHARED_PTR_LEAKS
            std::cout << "\n====================\n";
            std::cout << "found a leak:\n";
            auto cb = w.t_owner();
            auto tm = std::localtime(&cb->creation_time());
            std::cout << "in object: " << cb << " created on " << std::put_time(tm, "%c %Z") << " here:\n";
            std::cout << cb->creation_trace();
            std::cout << "\nwith living refs:\n";
            cb->log_active_strong(std::cout);
#else
            std::cout << "found a leak in " << w.lock() << "\n";
#endif
        }
    }
}
