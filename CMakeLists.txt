# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
cmake_minimum_required(VERSION 3.5)

project(xmem)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    # dev_mode is used below to make life easier for developers
    # it enables some configurations and the defaults for building tests and
    # examples which typically wouldn't be built if xmem is a subdirectory of
    # another project
    set(dev_mode ON)
else()
    set(dev_mode OFF)
endif()

option(XMEM_BUILD_TESTS "xmem: build tests" ${dev_mode})
option(XMEM_BUILD_EXAMPLES "xmem: build examples" ${dev_mode})
option(XMEM_BUILD_BENCHMARKS "xmem: build benchmarks" ${dev_mode})
mark_as_advanced(XMEM_BUILD_TESTS XMEM_BUILD_EXAMPLES XMEM_BUILD_BENCHMARKS)

if(dev_mode)
    include(./dev.cmake)
endif()

add_library(xmem INTERFACE)
add_library(xmem::xmem ALIAS xmem)
target_include_directories(xmem INTERFACE code)

add_library(xmem-test-helper INTERFACE)
add_library(xmem::test-helper ALIAS xmem-test-helper)
target_include_directories(xmem-test-helper INTERFACE test-helper)

if(XMEM_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

if(XMEM_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()

if(XMEM_BUILD_BENCHMARKS)
    add_subdirectory(benchmark)
endif()

if(dev_mode)
    add_subdirectory(scratch)
endif()
