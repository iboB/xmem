// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//

// inline file - no include guard

#include <doctest/util/lifetime_counter.hpp>

#include <cstdint>
#include <string>
#include <vector>

struct obj : public doctest::util::lifetime_counter<obj> {
    int a = 11;
    std::string b;

    obj() = default;
    explicit obj(int a) : a(a) {}
    obj(int a, std::string b) : a(a), b(b) {}
    virtual ~obj() = default;
    virtual int val() const { return a; }
};

struct child : obj {
    int c;
    child(int a, int c) : obj(a), c(c) {}
    virtual int val() const override { return a + c; }
};

struct cnt_deleter {
    intptr_t dels = 0;
    void operator()(int* iptr) {
        ++dels;
        delete iptr;
    }
    void operator()(obj* optr) {
        ++dels;
        delete optr;
    }
};

#if !defined(XMEM_TEST_NAMESPACE)
#   error "XMEM_TEST_NAMESPACE is not defined"
#endif
namespace XMEM_TEST_NAMESPACE {}
namespace test = XMEM_TEST_NAMESPACE;

#if !defined(ENABLE_XMEM_SPECIFIC_CHECKS)
#   error "ENABLE_XMEM_SPECIFIC_CHECKS is not defined"
#endif
#if ENABLE_XMEM_SPECIFIC_CHECKS
#   define XMEM(...) __VA_ARGS__
#   define STD20(...) __VA_ARGS__
#else
#   define XMEM(...)
#if __cplusplus >= 202000
#   define STD20(...) __VA_ARGS__
#else
#   define STD20(...)
#endif
#endif
