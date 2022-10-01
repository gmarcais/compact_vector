![CI](https://github.com/gmarcais/compact_vector/actions/workflows/c-cpp.yml/badge.svg?event=push)

# Compact vector

This library provide a bit-packed vector like data structure for
storing integer types.

* The number of bits used by each entry is specified either at
  compile time as a template argument, or at runtime. More
  optimizations are performed when the number of bits is known at
  compile time.
* The vector supports storing both signed and unsigned integer types.
* The library provides two variant of vectors (`vector` and
  `ts_vector`) with different multi-threading guarantees.

# Installation

## Inside a project

Copy the `.hpp` files from the `include` directory into your own
development tree. This is a header only library, no compilation is
necessary.

## Globally

To install the library globally on a system, use:

``` Shell
./configure --prefix=...
make install
```

After installation, use `pkg-config --cflags compact_vector` to use
the library.

## Testing

To run the unit tests of the library do

``` Shell
./configure
make check
```

# Usage

A simple partial example:
```C++
#include <compact_vector/compact_vector.hpp>

// Vector of signed int, using 6 bits
compact::vector<int, 6> ary;

// Vector of unsigned int, using a number of bits given from the command line
unsigned bits = atoi(argv[1]);
compact::vector<unsigned int> ary(bits);
```

The rest of the interface should be identical to the `vector` class
from the standard library.

The type `compact::vector<unsigned,1>` behaves equivalently to the
type `std::vector<bool>`. That is, it is a bit-packed vector where
each entry takes 1 bit of space.

# Multi-thread guarantees

Most operations on the `vector` class of the standard library (such as
`push_back`), are not thread safe. A few operations, like accessing
different entries of the vectors, are guaranteed to be thread safe.

The `compact_vector` library offers bit-packed data structure with and
without data access guarantees similar to the non-bit packed `vector`
class from the standard library.

## Bit-packed vectors

The `vector` class from the standard library has the following
guarantee for multi-threaded code: if `i != j`, then accessing `a[i]`
and `a[j]` from 2 different threads is safe.

On the other hand, this does not hold for the specialized
`vector<bool>` class, which is bit-packed. Accessing `a[i]` and `a[j]`
from two different threads is not safe, even if `i` and `j` are not
equal, because both values maybe stored in the same word.

The same apply to `compact::vector`, it is not safe to access elements
from two different threads.

## Thread safe vector class

The class `compact::ts_vector` is thread safe in the sense that if `i
!= j`, then accessing `a[i]` and `a[j]` from two different threads is
safe (just like `std::vector`). There is a performance penalties as
updates to the data structure are now performed with CAS (compare and
swap) atomic operations.
