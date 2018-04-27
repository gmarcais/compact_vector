#ifndef __COMPACT_INDEX_H__
#define __COMPACT_INDEX_H__

#ifdef HAVE_CONFIG
#include <config.h>
#endif
#ifdef HAVE_NUMA_H
#include <numa.h>
#endif
#include <unistd.h>

#include <new>
#include <vector>
#include <thread>

#include "compact_iterator.hpp"

namespace compact {

template<typename IDX, typename W = uint64_t, unsigned int UB = bitsof<W>::val>
struct index {
  const size_t       size;      // Size in number of element
  const unsigned int bits;      // Number of bits in an element
  W*                 mem;

  static inline int clz(unsigned int x) { return __builtin_clz(x); }
  static inline int clz(unsigned long x) { return __builtin_clzl(x); }
  static inline int clz(unsigned long long x) { return __builtin_clzll(x); }

  // Number of bits required for indices/values in the range [0, s).
  static unsigned int required_bits(size_t s) {
    unsigned int res = bitsof<size_t>::val - 1 - clz(s);
    res += (s > ((size_t)1 << res)) + (std::is_signed<IDX>::value ? 1 : 0);
    return res;
  }

  static size_t elements_to_words(size_t size, unsigned int bits) {
    size_t total_bits = size * bits;
    return total_bits / UB + (total_bits % UB != 0);
  }

  static void touch_mem(W* ptr, size_t pg_start, size_t pg_end) {
    const int words_per_page = getpagesize() / sizeof(W);
    for(W* p = ptr + words_per_page * pg_start; p < ptr + words_per_page * pg_end; p += words_per_page)
      *p = 0;
  }

  static W* alloc_mem(size_t s) {
#ifdef HAVE_NUMA_H
    W* res = (W*)numa_alloc_interleaved(sizeof(W) * s);
    if(!res) throw std::bad_alloc();
    size_t words_per_page = getpagesize() / sizeof(W);
    size_t number_of_pages = s  / words_per_page + (s % words_per_page != 0);
    std::vector<std::thread> threads;
    for(int i = 0; i < numa_max_node(); ++i)
      threads.push_back(std::thread(touch_mem, res,
                                    number_of_pages * i / numa_max_node(),
                                    number_of_pages * (i + 1) / numa_max_node()));
    for(int i = 0; i < numa_max_node(); ++i)
      threads[i].join();
    return res;
#else
    return new W[s];
#endif
  }

  typedef compact::iterator<IDX, W, false, UB> iterator;
  typedef compact::const_iterator<IDX, W, UB>  const_iterator;
  typedef compact::iterator<IDX, W, true, UB>  mt_iterator; // Multi thread safe version

  index(size_t s, unsigned int b) : size(s), bits(b), mem(alloc_mem(elements_to_words(size, bits))) { }
  explicit index(size_t s) :
    size(s),
    bits(required_bits(s)),
    mem(alloc_mem(elements_to_words(size, bits)))
  { }
  ~index() {
#ifdef HAVE_NUMA_H
    numa_free(mem, sizeof(W) * elements_to_words(size, bits));
#else
    delete [] mem;
#endif
  }

  const_iterator begin() const { return const_iterator(mem, bits, 0); }
  iterator begin() { return iterator(mem, bits, 0); }
  const_iterator end() const { return begin() + size; }
  iterator end() { return begin() + size; }
  const_iterator cbegin() const { return begin(); }
  const_iterator cend() const { return end(); }

  // Multi thread safe iterator
  mt_iterator mt_begin() { return mt_iterator(mem, bits, 0); }
  mt_iterator mt_end() { return begin() + size; }

  IDX operator[](size_t i) const { return cbegin()[i]; }

  W* get() { return mem; }
  const W* get() const { return mem; }
  size_t bytes() const { return sizeof(W) * elements_to_words(size, bits); }
};

} // namespace compact

#endif /* __COMPACT_INDEX_H__ */
