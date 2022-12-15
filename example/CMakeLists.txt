# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
include(./get_cpm.cmake)


macro(xmem_example name)
    cmake_parse_arguments(ARG "" "" "SOURCES;LIBS;INC" ${ARGN})
    set(tgt example-xmem-${name})
    add_executable(${tgt})
    target_sources(${tgt} PRIVATE ${ARG_SOURCES} ${ARG_UNPARSED_ARGUMENTS})
    target_link_libraries(${tgt} xmem::xmem ${ARG_LIBS})
    if(DEFINED ARG_INC)
        target_include_directories(${tgt} PRIVATE ${ARG_INC})
    endif()
endmacro()

xmem_example(unique_ptr-cppref e-unique_ptr-cppref.cpp)

CPMAddPackage(gh:iboB/b_stacktrace@0.10)

xmem_example(shared_ptr-leak
    e-shared_ptr-leak.cpp
    INC ${b_stacktrace_SOURCE_DIR}
)
