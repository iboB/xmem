## TODO

* tracking demo
* allocator-based tests of `shared_ptr`
* allocate_unique
* wrappers for the standard `<memory>` types

## Differences

* `std::shared_ptr`
    * Custom owner
    * Getter for owner
    * No ctor from weak_ptr
    * `make_aliased`
    * `make_ptr`
    * no atomic ops
* `std::weak_ptr`
    * Custom owner
    * Getter for owner
    * `bool` interface (on null owner)
    * aliasing ctor
* `shared_from`
* `shared_from_this` - no exception
* `same_owner`, `no_woner`
* `atomic_shared_ptr_storage`
* no exceptions as a whole
* C++20 features

