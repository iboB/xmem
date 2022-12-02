// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//

// inline file - no include guard

#include <cstdint>
#include <string>

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
