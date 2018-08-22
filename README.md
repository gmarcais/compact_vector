# Compact vector

This library provide a bit-packed vector like data structure.

# Installation

The simple way is to copy the `.hpp` files in `include` into your own development tree.
This is a header only library, no compilation is necessary.

To run the unit tests of the library, first configure with `./configure`, then run the tests with `make check`.

# Usage

A simple partial example:
```C++
#include <compact_vector.hpp>

compact::vector<int> ary(6);
```
This create a variable `ary` storing signed integers that are 6 bit long.
The interface is otherwise (mostly) compatible with the vector class.

Other classes, `compact::ts_vector` and `compact::mt_vector` give different multi-threading guarantees.
