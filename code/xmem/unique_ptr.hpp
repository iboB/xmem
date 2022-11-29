// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once

#include <cstddef> // for nullptr_t
#include <utility> // for std::forward at the bottom

namespace xmem {

// trivial unique pointer class to guarantee that the destructor of the managed object
// is called before the pointer itself is ivalidated
// this allows complex self-referencing from the managed object
// we need this because not all standard library implementation have this guarantee :(
// more here: https://ibob.bg/blog/2019/11/07/dont-use-unique_ptr-for-pimpl/
template <typename T>
class unique_ptr
{
public:
    using pointer = T*;

    unique_ptr() noexcept = default;
    unique_ptr(std::nullptr_t) noexcept {}
    explicit unique_ptr(pointer p) noexcept : m_ptr(p) {}
    unique_ptr(unique_ptr&& other) noexcept : m_ptr(other.release()) {}
    unique_ptr(const unique_ptr&) = delete;

    ~unique_ptr() { delete m_ptr; }

    unique_ptr& operator=(unique_ptr&& other) noexcept
    {
        reset(other.release());
        return *this;
    }
    unique_ptr& operator=(std::nullptr_t) noexcept
    {
        reset();
        return *this;
    }
    unique_ptr& operator=(const unique_ptr&) = delete;

    pointer release()
    {
        auto old = m_ptr;
        m_ptr = nullptr;
        return old;
    }

    void reset(pointer p = nullptr)
    {
        auto old = m_ptr;
        m_ptr = p;
        delete old;
    }

    explicit operator bool() const noexcept { return !!m_ptr; }

    pointer get() const noexcept { return m_ptr; }
    T& operator*() const { return *m_ptr; }
    T* operator->() const noexcept { return m_ptr; }

private:
    pointer m_ptr = nullptr;
};

template <typename T, typename... Args>
unique_ptr<T> make_unique(Args&&... args)
{
    return unique_ptr<T>{new T(std::forward<Args>(args)...)};
}

template <typename T>
auto make_unique_ptr(T&& t) -> unique_ptr<typename std::remove_reference<T>::type>
{
    using RRT = typename std::remove_reference<T>::type;
    return  unique_ptr<RRT>(new RRT(std::forward<T>(t)));
}

}