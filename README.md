# TA memory allocator

[![Build](https://img.shields.io/github/actions/workflow/status/pavelxdd/ta/cmake.yml?branch=master&style=flat)](https://github.com/pavelxdd/ta/actions)
[![Codecov](https://img.shields.io/codecov/c/gh/pavelxdd/ta?style=flat)](https://codecov.io/gh/pavelxdd/ta)
[![License](https://img.shields.io/github/license/pavelxdd/ta?style=flat&color=blue)](https://github.com/pavelxdd/ta/blob/master/UNLICENSE)

TA (Tree Allocator) is a hierarchical memory allocator with destructors.

Essentially it is a wrapper around malloc and related functions.
When a parent allocation is freed all child memory allocations are automatically freed too.

Unlike standard malloc, allocation size may be zero, in which case there is an empty
allocation which can still be used as a parent for other allocations.

````bash
cmake . -Bbuild
cmake --build build
````

References:

- [samba talloc](https://talloc.samba.org/talloc/doc/html/index.html)
- [mpv talloc](https://github.com/mpv-player/mpv/tree/master/ta)
