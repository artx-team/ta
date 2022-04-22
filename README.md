# TA memory allocator

TA ("Tree Allocator") is a hierarchical memory allocator with destructors.

Essentially it is a wrapper around `malloc()` and related functions.
When a parent allocation is freed all child memory allocations are automatically freed too.

Unlike standard malloc, allocation size may be zero, in which case there is an empty
allocation which can still be used as a parent for other allocations.

````bash
cmake . -Bbuild 
cmake --build build
````

References:

- [mpv talloc](https://github.com/mpv-player/mpv/tree/master/ta)
- [samba talloc](https://talloc.samba.org/talloc/doc/html/index.html)
