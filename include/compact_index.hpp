#ifndef __COMPACT_INDEX_H__
#define __COMPACT_INDEX_H__

#ifdef HAVE_CONFIG
#include <config.h>
#endif

#include <new>
#include <vector>
#include <stdexcept>

#include "compact_iterator.hpp"

namespace compact {

namespace index_imp {

template<typename D, typename IDX, typename W, typename Allocator, unsigned int UB, bool TS>
class index {
  const size_t       m_size;      // Size in number of element
  const unsigned int m_bits;      // Number of bits in an element
  W*                 mem;

  static inline int clz(unsigned int x) { return __builtin_clz(x); }
  static inline int clz(unsigned long x) { return __builtin_clzl(x); }
  static inline int clz(unsigned long long x) { return __builtin_clzll(x); }

public:
  // Number of bits required for indices/values in the range [0, s).
  static unsigned int required_bits(size_t s) {
    unsigned int res = bitsof<size_t>::val - 1 - clz(s);
    res += (s > ((size_t)1 << res)) + (std::is_signed<IDX>::value ? 1 : 0);
    return res;
  }

  static size_t elements_to_words(size_t size, unsigned int bits) {
    size_t total_bits = size * bits;
    return total_bits / D::used_bits + (total_bits % D::used_bits != 0);
  }

  typedef compact::iterator<IDX, W, TS, UB> iterator;
  typedef compact::const_iterator<IDX, W, UB>  const_iterator;
  typedef compact::iterator<IDX, W, true, UB>  mt_iterator; // Multi thread safe version

  index(size_t s, unsigned int b)
    : m_size(s)
    , m_bits(b)
    , mem(new W[elements_to_words(m_size, m_bits)]) {
    static_assert(D::used_bits <= bitsof<W>::val, "used_bits must be less or equal to the number of bits in the word_type");
    if(b > D::used_bits)
      throw std::out_of_range("Number of bits larger than usable bits");
  }
  explicit index(size_t s)
    : index(s, required_bits(s))
  { }
  ~index() {
    delete [] mem;
  }

  const_iterator begin() const { return const_iterator(mem, m_bits, 0); }
  iterator begin() { return iterator(mem, m_bits, 0); }
  const_iterator end() const { return begin() + m_size; }
  iterator end() { return begin() + m_size; }
  const_iterator cbegin() const { return begin(); }
  const_iterator cend() const { return end(); }

  // Multi thread safe iterator
  mt_iterator mt_begin() { return mt_iterator(mem, m_bits, 0); }
  mt_iterator mt_end() { return begin() + m_size; }

  IDX operator[](size_t i) const { return cbegin()[i]; }

  size_t size() const { return m_size; }
  unsigned int bits() const { return m_bits; }

  W* get() { return mem; }
  const W* get() const { return mem; }
  size_t bytes() const { return sizeof(W) * elements_to_words(m_size, m_bits); }
};
} // namespace index_imp

template<typename IDX, typename W = uint64_t, typename Allocator = std::allocator<W>,
         unsigned int UB = bitsof<W>::val>
class vector
  : public index_imp::index<vector<IDX,W,Allocator,UB>, IDX, W, Allocator, UB, false>
{
  typedef index_imp::index<vector<IDX,W,Allocator,UB>, IDX, W, Allocator, UB, false> super;
public:
  typedef IDX                                  value_type;
  typedef W                                    word_type;
  static constexpr unsigned int                used_bits = UB;

  typedef typename super::iterator iterator;
  typedef typename super::const_iterator const_iterator;
  typedef typename super::mt_iterator mt_iterator;

  vector(size_t s, size_t b)
    : super(s, b)
  { }
  vector(size_t s)
    : super(s)
  { }
};

template<typename IDX, typename W = uint64_t, typename Allocator = std::allocator<W>,
         unsigned int UB = bitsof<W>::val>
class mt_vector
  : public index_imp::index<vector<IDX,W,Allocator,UB>, IDX, W, Allocator, UB, true>
{
  typedef index_imp::index<vector<IDX,W,Allocator,UB>, IDX, W, Allocator, UB, true> super;
public:
  typedef IDX                                  value_type;
  typedef W                                    word_type;
  static constexpr unsigned int                used_bits = UB;

  typedef typename super::iterator iterator;
  typedef typename super::const_iterator const_iterator;
  typedef typename super::mt_iterator mt_iterator;

  mt_vector(size_t s, size_t b)
    : super(s, b)
  { }
  mt_vector(size_t s)
    : super(s)
  { }
};



} // namespace compact

#endif /* __COMPACT_INDEX_H__ */
