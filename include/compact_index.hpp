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

template<typename IDX, typename W, typename Allocator, unsigned int UB, bool TS>
class index {
  Allocator          m_allocator;
  const size_t       m_size;      // Size in number of element
  const unsigned int m_bits;      // Number of bits in an element
  W*                 m_mem;

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
    return total_bits / UB + (total_bits % UB != 0);
  }

  typedef compact::iterator<IDX, W, TS, UB> iterator;
  typedef compact::const_iterator<IDX, W, UB>  const_iterator;
  typedef compact::iterator<IDX, W, true, UB>  mt_iterator; // Multi thread safe version

  index(size_t s, unsigned int b, Allocator allocator = Allocator())
    : m_allocator(allocator)
    , m_size(s)
    , m_bits(b)
    , m_mem(m_allocator.allocate(elements_to_words(m_size, m_bits))) {
    static_assert(UB <= bitsof<W>::val, "used_bits must be less or equal to the number of bits in the word_type");
    if(b > UB)
      throw std::out_of_range("Number of bits larger than usable bits");
  }
  explicit index(size_t s)
    : index(s, required_bits(s))
  { }
  ~index() {
    m_allocator.deallocate(m_mem, elements_to_words(m_size, m_bits));
  }

  const_iterator begin() const { return const_iterator(m_mem, m_bits, 0); }
  iterator begin() { return iterator(m_mem, m_bits, 0); }
  const_iterator end() const { return begin() + m_size; }
  iterator end() { return begin() + m_size; }
  const_iterator cbegin() const { return begin(); }
  const_iterator cend() const { return end(); }

  // Multi thread safe iterator
  mt_iterator mt_begin() { return mt_iterator(m_mem, m_bits, 0); }
  mt_iterator mt_end() { return begin() + m_size; }

  IDX operator[](size_t i) const { return cbegin()[i]; }

  size_t size() const { return m_size; }
  unsigned int bits() const { return m_bits; }

  W* get() { return m_mem; }
  const W* get() const { return m_mem; }
  size_t bytes() const { return sizeof(W) * elements_to_words(m_size, m_bits); }
};
} // namespace index_imp

template<typename IDX, typename W = uint64_t, typename Allocator = std::allocator<W>>
class vector
  : public index_imp::index<IDX, W, Allocator, bitsof<W>::val, false>
{
  typedef index_imp::index<IDX, W, Allocator, bitsof<W>::val, false> super;
public:
  typedef typename super::iterator              iterator;
  typedef typename super::const_iterator        const_iterator;
  typedef IDX                                   value_type;
  typedef Allocator                             allocator_type;
  typedef typename iterator::setter_type        reference;
  typedef const reference                       const_reference;
  typedef iterator                              pointer;
  typedef const_iterator                        const_pointer;
  typedef std::reverse_iterator<iterator>       reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
  typedef ptrdiff_t                             difference_type;
  typedef size_t                                size_type;
  typedef W                                     word_type;
  static constexpr unsigned int                 used_bits = bitsof<W>::val;

  vector(size_t s, size_t b)
    : super(s, b)
  { }
  vector(size_t s)
    : super(s)
  { }
};

//unsigned int UB = bitsof<W>::val>
template<typename IDX, typename W = uint64_t, typename Allocator = std::allocator<W>>
class ts_vector
  : public index_imp::index<IDX, W, Allocator, bitsof<W>::val, true>
{
  typedef index_imp::index<IDX, W, Allocator, bitsof<W>::val, true> super;
public:
  typedef typename super::iterator              iterator;
  typedef typename super::const_iterator        const_iterator;
  typedef IDX                                   value_type;
  typedef Allocator                             allocator_type;
  typedef typename iterator::setter_type        reference;
  typedef const reference                       const_reference;
  typedef iterator                              pointer;
  typedef const_iterator                        const_pointer;
  typedef std::reverse_iterator<iterator>       reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
  typedef ptrdiff_t                             difference_type;
  typedef size_t                                size_type;
  typedef W                                     word_type;
  static constexpr unsigned int                 used_bits = bitsof<W>::val;

  ts_vector(size_t s, size_t b)
    : super(s, b)
  { }
  ts_vector(size_t s)
    : super(s)
  { }
};

template<typename IDX, typename W = uint64_t, typename Allocator = std::allocator<W>>
class cas_vector
  : public index_imp::index<IDX, W, Allocator, bitsof<W>::val - 1, true>
{
  typedef index_imp::index<IDX, W, Allocator, bitsof<W>::val - 1, true> super;
public:
  typedef typename super::iterator              iterator;
  typedef typename super::const_iterator        const_iterator;
  typedef IDX                                   value_type;
  typedef Allocator                             allocator_type;
  typedef typename iterator::setter_type        reference;
  typedef const reference                       const_reference;
  typedef iterator                              pointer;
  typedef const_iterator                        const_pointer;
  typedef std::reverse_iterator<iterator>       reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
  typedef ptrdiff_t                             difference_type;
  typedef size_t                                size_type;
  typedef W                                     word_type;
  static constexpr unsigned int                 used_bits = bitsof<W>::val - 1;

  cas_vector(size_t s, size_t b)
    : super(s, b)
  { }
  cas_vector(size_t s)
    : super(s)
  { }
};



} // namespace compact

#endif /* __COMPACT_INDEX_H__ */
