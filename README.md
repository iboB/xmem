# xmem

An alternative implementation of (most of) the functionality of the standard header `<memory>` with some additional features.

xmem requires at least C++17.

## xmem vs &lt;memory&gt;

* `unique_ptr`: no known differences
* `shared_ptr`:
    * xmem allows the control block type to be set as a template argument, thus allowing various features by control-block-level polymorphism.
        * Most notably (and the main motivation behind the lib): tracking of strong or weak refs, which allow dealing with `shared_ptr`-based leaks
        * Non-atomic refcounts which improve performance as long as you only use the pointers in a single thread (implemented in `xmem::local_shared_ptr`)
    * In the spirit of the deprecated atomic operations on `std::shared_ptr` in C++20, xmem offers no atomic ops on `shared_ptr`. It introduces the class `atomic_shared_ptr_storage` to take care of this need.
    * The owner (control block) of the pointer is directly accessible as `const void*` through `ptr.owner()` and stronly typed as `const control_block_type*` through `ptr.t_owner()`
    * There is no constructor through weak ptr, and no `shared_ptr` operation throws an exception (except ones by proxy, on allocation or if constructing the object in `make_shared` throw)
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

The external functionalities: `make_aliased`, `make_ptr`, `enable_shared_from`, `atomic_shared_ptr_storage` are also available for `std::shared_ptr` through the header `xmem/std_helpers.hpp`

## TODO

This is not yet production ready, though it is likely usable. Here are the most important things to do before I can call it production ready:

* Create tracking demos, and perhaps improve the interface to support them better
* Write `allocator`-based tests for `shared_ptr`, and fix bugs if any
* Create benchmakrs and look for peformance issues

## License

This software is distributed under the MIT Software License.

See accompanying file LICENSE or copy [here](https://opensource.org/licenses/MIT).

Copyright &copy; 2022 [Borislav Stanimirov](http://github.com/iboB)