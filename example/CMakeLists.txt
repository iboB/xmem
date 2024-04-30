# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
cmake_minimum_required(VERSION 3.14)
include(../cmake/get_cpm.cmake)

find_package(Threads REQUIRED)

macro(xmem_example name)
    cmake_parse_arguments(ARG "" "" "SOURCES;LIBS;INC" ${ARGN})
    set(tgt example-xmem-${name})
    add_executable(${tgt})
    target_sources(${tgt} PRIVATE ${ARG_SOURCES} ${ARG_UNPARSED_ARGUMENTS})
    target_link_libraries(${tgt} xmem::xmem ${ARG_LIBS} ${CMAKE_THREAD_LIBS_INIT})
    if(DEFINED ARG_INC)
        target_include_directories(${tgt} PRIVATE ${ARG_INC})
    endif()
endmacro()

xmem_example(unique_ptr-cppref e-unique_ptr-cppref.cpp)
xmem_example(shared_ptr-cppref e-shared_ptr-cppref.cpp)
xmem_example(local_shared_ptr-cppref e-local_shared_ptr-cppref.cpp)

CPMAddPackage(gh:iboB/b_stacktrace@0.22)

xmem_example(shared_ptr-leak
    e-shared_ptr-leak.cpp
    LIBS ${CMAKE_DL_LIBS} b_stacktrace::b_stacktrace
)
