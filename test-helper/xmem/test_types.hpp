#pragma once
#include <doctest/util/lifetime_counter.hpp>
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

struct alignas(64) avx_512 { double d[8]; };

struct vec { int x; int y; };

struct multi_child : obj, vec {
    int z = 20;
};
