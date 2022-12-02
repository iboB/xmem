# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
cmake_minimum_required(VERSION 3.14)

include(./get_cpm.cmake)

CPMAddPackage(gh:iboB/doctest-util@0.0.5)

macro(xmem_test test)
    add_doctest_lib_test(${test} xmem ${ARGN} LIBRARIES xmem::test-helper)
endmacro()

xmem_test(unique_ptr t-unique_ptr.cpp)
xmem_test(local_shared_ptr t-local_shared_ptr.cpp)
