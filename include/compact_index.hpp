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

template<typename IDX, typename W=uint64_t>
struct compact_index {
  const size_t       size;      // Size in number of element
  const unsigned int bits;      // Number of bits in an element
  W*                 mem;

  static unsigned int required_bits(size_t s) {
    unsigned int res = 63 - __builtin_clzl(s);
    res += (s > ((size_t)1 << res)) + (std::is_signed<IDX>::value ? 1 : 0);
    return res;
  }

  static size_t elements_to_words(size_t size, unsigned int bits) {
    static const size_t wbits = 8 * sizeof(W);
    size_t total_bits = size * bits;
    return total_bits / wbits + (total_bits % wbits != 0);
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

  typedef compact_iterator<IDX, W>       iterator;
  typedef const_compact_iterator<IDX, W> const_iterator;

  compact_index(size_t s, unsigned int b) : size(s), bits(b), mem(alloc_mem(elements_to_words(size, bits))) { }
  explicit compact_index(size_t s) :
    size(s),
    bits(required_bits(s)),
    mem(alloc_mem(elements_to_words(size, bits)))
  { }
  ~compact_index() {
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

  IDX operator[](size_t i) const { return cbegin()[i]; }

  W* get() { return mem; }
  const W* get() const { return mem; }
  size_t bytes() const { return sizeof(W) * elements_to_words(size, bits); }
};

#endif /* __COMPACT_INDEX_H__ */
