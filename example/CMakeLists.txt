# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
macro(xmem_example name)
    set(tgt example-xmem-${name})
    add_executable(${tgt})
    target_sources(${tgt} PRIVATE ${ARGN})
    target_link_libraries(${tgt} xmem::xmem)
endmacro()

xmem_example(unique_ptr-cppref e-unique_ptr-cppref.cpp)
