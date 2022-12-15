// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//

// inline file - no include guard

#include "test_types.hpp"

#include <cstdint>
#include <vector>

#include <splat/warnings.h>
DISABLE_CLANG_ONLY_WARNING("-Wself-assign-overloaded")

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

#if defined(XMEM_XSTD_NAMESPACE)
namespace XMEM_XSTD_NAMESPACE {}
namespace xtest = XMEM_XSTD_NAMESPACE;
#else
namespace xtest = test;
#endif


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
