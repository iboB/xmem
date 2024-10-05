# xmem

[![Standard](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization) [![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://opensource.org/licenses/MIT) [![Build](https://github.com/iboB/xmem/actions/workflows/unit-test.yml/badge.svg)](https://github.com/iboB/xmem/actions/workflows/unit-test.yml)

An alternative implementation of the smart pointers in the standard header `<memory>` with some additional features.

[Blog post about this library and the motivation behind it.](https://ibob.bg/blog/2023/01/01/tracking-shared-ptr-leaks/)

xmem requires at least C++17.

## xmem vs &lt;memory&gt;

* `unique_ptr`: no known differences
* `shared_ptr`:
    * xmem allows the control block type to be set as a template argument, thus allowing various features by control-block-level polymorphism.
        * Most notably (and the main motivation behind the lib): tracking of strong or weak refs, which allow dealing with `shared_ptr`-based leaks
        * Non-atomic refcounts which improve performance as long as you only use the pointers in a single thread (implemented in `xmem::local_shared_ptr`)
    * In the spirit of the deprecated atomic operations on `std::shared_ptr` in C++20, xmem offers no atomic ops on `shared_ptr`. It introduces the class `atomic_shared_ptr_storage` to take care of this need.
    * The owner (control block) of the pointer is directly accessible as `const void*` through `ptr.owner()` and stronly typed as `const control_block_type*` through `ptr.t_owner()`
    * There is no constructor through weak ptr, and no `shared_ptr` operation throws an exception (except ones by proxy, on allocation or if constructing the object in `make_shared` throws)
    * A helper function: `make_shared_ptr` to make a `shared_ptr` from an existing object
    * A helper function: `make_aliased` to make a `shared_ptr` by aliasing another, but safely returning `nullptr` if the source is null.
* `weak_ptr`:
    * Like `shared_ptr` it has the control block as a template argument and offers control block access through `owner` and `t_owner`
    * The pointer has a boolean interface which means no associated control block and says nothing about whether the pointer has expired or not.
    * An aliasing constructor like the one in `shared_ptr` is provided
* Besides `enable_shared_from_this` a non-template alternative is introduced `enable_shared_from` which (in the author's opinion) has a better interface. This is inspired from Boost.SmartPtr.
* Helper functions
    * `same_owner` - check whether two shared/weak pointers have the same owner
    * `no_owner` - check if a weak/shared pointer has no owner
* Functions from C++20: `make_shared_for_overwrite`, `make_unique_for_overwrite`

The external functionalities: `make_aliased`, `make_ptr`, `enable_shared_from`, `atomic_shared_ptr_storage`, `same_owner`, `no_owner`, are also available for the applicable `std::` pointers through the header `xmem/std_helpers.hpp` in namespace `xstd`.

## License

This software is distributed under the MIT Software License.

See accompanying file LICENSE or copy [here](https://opensource.org/licenses/MIT).

Copyright &copy; 2022-2024 [Borislav Stanimirov](http://github.com/iboB)
