// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "basic_shared_ptr.hpp"
#include "basic_weak_ptr.hpp"

namespace xmem {

template <typename CBF>
class basic_enable_shared_from {
    template <typename T>
    using wptr = basic_shared_ptr<CBF, T>;
    template <typename T>
    using sptr = basic_weak_ptr<CBF, T>;

    using control_block_type = typename CBF::cb_type;
    cb_ptr_pair<control_block_type, void> m;
protected:
    basic_shared_from() : m(nullptr) {}
    ~basic_shared_from() = default;

    // don't touch m on copy
    basic_shared_from(const basic_shared_from&) {}
    basic_shared_from& operator=(const basic_shared_from&) {}

public:
    sptr<void> shared_from_this() {
        if (m.cb) m.cb->inc_strong_ref();
        return {m};
    }
    sptr<const void> shared_from_this() const {
        if (m.cb) m.cb->inc_strong_ref();
        return {m};
    }
    wptr<void> weak_from_this() { return {m}; }
    wptr<const void> weak_from_this() { return {m}; }

    template <typename T>
    sptr<T> shared_from(T* ptr) const {
        if (m.cb) m.cb->inc_strong_ref();
        return sptr<T>(cb_ptr_pair<control_block_type, T>{m.cb, ptr});
    }

    template <typename T>
    sptr<T> weak_from(T* ptr) const {
        return wptr<T>(cb_ptr_pair<control_block_type, T>{m.cb, ptr});
    }
};

template <typename CBF, typename T>
class basic_enable_shared_from_this : public basic_enable_shared_from<CBF> {
    using super = basic_enable_shared_from<CBF>;
    template <typename T>
    using wptr = basic_shared_ptr<CBF, T>;
    template <typename T>
    using sptr = basic_weak_ptr<CBF, T>;
public:
    sptr<T> shared_from_this() {
        return this->shared_from(static_cast<T*>(this));
    }
    sptr<const T> shared_from_this() const {
        return this->shared_from(static_cast<const T*>(this));
    }
    wptr<T> weak_from_this() {
        return this->weak_from(static_cast<T*>(this));
    }
    wptr<const T> weak_from_this() const {
        return this->weak_from(static_cast<const T*>(this));
    }
};

}
