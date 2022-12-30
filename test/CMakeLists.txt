# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
cmake_minimum_required(VERSION 3.14)
include(../cmake/get_cpm.cmake)

CPMAddPackage(gh:iboB/doctest-util@0.0.7)
CPMAddPackage(gh:iboB/splat@1.2.2)

find_package(Threads REQUIRED)

macro(xmem_test test)
    add_doctest_lib_test(${test} xmem ${ARGN} LIBRARIES xmem::test-helper splat::splat ${CMAKE_THREAD_LIBS_INIT})
endmacro()

xmem_test(unique_ptr t-unique_ptr.cpp)
xmem_test(sanity_std_unique_ptr t-sanity_std_unique_ptr.cpp)

xmem_test(allocator t-allocator.cpp)
xmem_test(allocator_std_compat t-allocator_std_compat.cpp)

xmem_test(local_ref_count t-local_ref_count.cpp)
xmem_test(local_shared_ptr t-local_shared_ptr.cpp)

xmem_test(shared_ptr_local_bk t-shared_ptr_local_bk.cpp)

xmem_test(atomic_ref_count t-atomic_ref_count.cpp)
xmem_test(shared_ptr t-shared_ptr.cpp)

xmem_test(shared_ptr_mt_bk t-shared_ptr_mt_bk.cpp)

xmem_test(sanity_std_shared_ptr t-sanity_std_shared_ptr.cpp)