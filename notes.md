## TODO

* atomic ref count
* tracking demo
* allocator-based tests of `shared_ptr`
* allocate_unique

## Differences

* `std::shared_ptr`
    * Custom owner
    * Getter for owner
    * No ctor from weak_ptr
* `std::weak_ptr`
    * Custom owner
    * Getter for owner
    * `bool` interface (on null owner)
* `shared_from`
* `shared_from_this` - no exception
* no exceptions as a whole
