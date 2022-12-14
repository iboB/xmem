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
    using wptr = basic_weak_ptr<CBF, T>;
    template <typename T>
    using sptr = basic_shared_ptr<CBF, T>;

    friend CBF;

    using control_block_type = typename CBF::cb_type;
    cb_ptr_pair<control_block_type, void> m;
protected:
    basic_enable_shared_from() : m(nullptr) {}
    ~basic_enable_shared_from() = default;

    // don't touch m on copy
    basic_enable_shared_from(const basic_enable_shared_from&) : m(nullptr) {}
    basic_enable_shared_from& operator=(const basic_enable_shared_from&) { return *this; }

    sptr<void> shared_from_this() {
        sptr<void> ret(m);
        if (m.cb && m.cb->inc_strong_ref_nz(&ret)) return ret;
        return {};
    }
    sptr<const void> shared_from_this() const {
        sptr<const void> ret(m);
        if (m.cb && m.cb->inc_strong_ref_nz(&ret)) return ret;
        return {};
    }
    wptr<void> weak_from_this() { return wptr<void>{m}; }
    wptr<const void> weak_from_this() const { return wptr<const void>{m}; }

    template <typename T>
    sptr<T> shared_from(T* ptr) const {
        cb_ptr_pair<control_block_type, T> alias{m.cb, ptr};
        sptr<T> ret{alias};
        if (m.cb && m.cb->inc_strong_ref_nz(&ret)) return ret;
        return {};
    }

    template <typename T>
    wptr<T> weak_from(T* ptr) const {
        if (!m.cb) return {};
        return wptr<T>(cb_ptr_pair<control_block_type, T>{m.cb, ptr});
    }
};

template <typename CBF, typename T>
class basic_enable_shared_from_this : public basic_enable_shared_from<CBF> {
    using super = basic_enable_shared_from<CBF>;
    template <typename PT>
    using wptr = basic_weak_ptr<CBF, PT>;
    template <typename PT>
    using sptr = basic_shared_ptr<CBF, PT>;
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
