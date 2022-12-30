# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
cmake_minimum_required(VERSION 3.14)
include(../cmake/get_cpm.cmake)

CPMAddPackage(gh:iboB/picobench@2.01)

add_library(picobench-main STATIC picobench-main.cpp)
target_link_libraries(picobench-main PUBLIC picobench)

macro(xmem_benchmark name)
    set(tgt bench-xmem-${name})
    add_executable(${tgt})
    target_sources(${tgt} PRIVATE ${ARGN})
    target_link_libraries(${tgt} xmem::xmem picobench-main)
    add_custom_target(
        benchmark-xmem-${name}
        COMMAND ${tgt}
    )
endmacro()

xmem_benchmark(unique_ptr b-unique_ptr-std.cpp b-unique_ptr-xmem.cpp)