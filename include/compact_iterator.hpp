#ifndef __COMPACT_ITERATOR_H__
#define __COMPACT_ITERATOR_H__

#include <iterator>
#include <memory>
#include <type_traits>
#include <cstddef>

#include "const_iterator_traits.hpp"
#include "parallel_iterator_traits.hpp"
#include "prefetch_iterator_traits.hpp"

template<typename IDX, typename W>
class const_compact_iterator;
template<typename IDX, typename W, bool TS>
class compact_iterator;

namespace compact_iterator_imp {

template<typename IDX, typename W>
static IDX get(const W* p, unsigned int b, unsigned int o) {
  static const size_t Wbits = sizeof(W) * 8;
  W mask = (~(W)0 >> (Wbits - b)) << o;
  IDX res = (*p & mask) >> o;
  if(o + b > Wbits) {
    unsigned int over = o + b - Wbits;
    mask = ~(W)0 >> (Wbits - over);
    res |= (*(p + 1) & mask) << (b - over);
  }
  if(std::is_signed<IDX>::value && res & ((IDX)1 << (b - 1)))
    res |= ~(IDX)0 << b;

  return res;
}

template<typename W, bool TS>
struct mask_store { };

template<typename W>
struct mask_store<W, false> {
  static inline void store(W* p, W mask, W val) {
    *p = (*p & ~mask) | (val & mask);
  }
};

template<typename W>
struct mask_store<W, true> {
  static void store(W* p, W mask, W val) {
    W cval = *p, oval;
    do {
      W nval = (cval & ~mask) | (val & mask);
      oval = cval;
      cval = __sync_val_compare_and_swap(p, oval, nval);
    } while(cval != oval);
  }
};

template<typename IDX, typename W, bool TS = false>
static void set(IDX x, W* p, unsigned int b, unsigned int o) {
  static const size_t Wbits = sizeof(W) * 8;
  const W y = x;
  W   mask  = (~(W)0 >> (Wbits - b)) << o;
  mask_store<W, TS>::store(p, mask, y << o);
  if(o + b > Wbits) {
    unsigned int over = o + b - Wbits;
    mask              = ~(W)0 >> (Wbits - over);
    mask_store<W, TS>::store(p + 1, mask, y >> (b - over));
  }
}

template<class Derived, typename IDX, typename W>
class common {
public:
  std::ostream& print(std::ostream& os) const {
    const Derived& self = *static_cast<const Derived*>(this);
    return os << '<' << (void*)self.ptr << '+' << self.offset << ',' << self.bits << '>';
  }

protected:
  static const unsigned int Wbits = sizeof(W) * 8;

public:
  typedef typename std::iterator<std::random_access_iterator_tag, IDX>::difference_type difference_type;

  Derived& operator=(const Derived& rhs) {
    Derived& self = *static_cast<Derived*>(this);
    self.ptr      = rhs.ptr;
    self.bits     = rhs.bits;
    self.offset   = rhs.offset;
    return self;
  }

  Derived& operator=(std::nullptr_t p) {
    Derived& self = *static_cast<Derived*>(this);
    self.ptr      = nullptr;
    self.offset   = 0;
  }

  IDX operator*() const {
    const Derived& self = *static_cast<const Derived*>(this);
    return get<IDX, W>(self.ptr, self.bits, self.offset);
  }

  bool operator==(const Derived& rhs) const {
    const Derived& self = *static_cast<const Derived*>(this);
    return self.ptr == rhs.ptr && self.offset == rhs.offset;
  }
  bool operator!=(const Derived& rhs) const {
    return !(*this == rhs);
  }

  bool operator==(std::nullptr_t p) {
    const Derived& self = *static_cast<const Derived*>(this);
    return self.ptr == nullptr && self.offset == 0;
  }
  bool operator!=(std::nullptr_t p) {
    return !(*this == nullptr);
  }

  bool operator<(const Derived& rhs) const {
    const Derived& self = *static_cast<const Derived*>(this);
    return self.ptr < rhs.ptr || (self.ptr == rhs.ptr && self.offset < rhs.offset);
  }
  bool operator>(const Derived& rhs) const {
    const Derived& self = *static_cast<const Derived*>(this);
    return self.ptr > rhs.ptr || (self.ptr == rhs.ptr && self.offset > rhs.offset);
  }
  bool operator>=(const Derived& rhs) const {
    return !(*this < rhs);
  }
  bool operator<=(const Derived& rhs) const {
    return !(*this > rhs);
  }

  Derived& operator++() {
    Derived& self = *static_cast<Derived*>(this);
    self.offset += self.bits;
    if(self.offset >= Wbits) {
      ++self.ptr;
      self.offset -= Wbits;
    }
    return self;
  }
  Derived operator++(int) {
    Derived res(*static_cast<Derived*>(this));
    ++*this;
    return res;
  }

  Derived& operator--() {
    Derived& self = *static_cast<Derived*>(this);
    if(self.bits > self.offset) {
      --self.ptr;
      self.offset += Wbits;
    }
    self.offset -= self.bits;
    return self;
  }
  Derived operator--(int) {
    Derived res(*static_cast<Derived*>(this));
    --*this;
    return res;
  }

  Derived& operator+=(difference_type n) {
    Derived&     self    = *static_cast<Derived*>(this);
    if(n < 0) {
      self -= -n;
      return self;
    }

    const size_t nbbits  = self.bits * n;
    self.ptr            += nbbits / Wbits;
    self.offset         += nbbits % Wbits;
    if(self.offset >= Wbits) {
      ++self.ptr;
      self.offset -= Wbits;
    }
    return self;
  }

  Derived operator+(difference_type n) const {
    Derived res(*static_cast<const Derived*>(this));
    return res += n;
  }

  Derived& operator-=(difference_type n) {
    Derived&           self     = *static_cast<Derived*>(this);
    if(n < 0) {
      self += -n;
      return self;
    }

    const size_t      nbbits    = self.bits * n;
    self.ptr                   -= nbbits / Wbits;
    const unsigned int ooffset  = nbbits % Wbits;
    if(ooffset > self.offset) {
      --self.ptr;
      self.offset += Wbits;
    }
    self.offset -= ooffset;
    return self;
  }

  Derived operator-(difference_type n) const {
    Derived res(*static_cast<const Derived*>(this));
    return res -= n;
  }

  template<typename DD, typename II, typename WW>
  difference_type operator-(const common<DD, II, WW>& rhs_) const {
    const Derived& self  = *static_cast<const Derived*>(this);
    const DD&      rhs   = *static_cast<const DD*>(&rhs_);
    ptrdiff_t      wdiff = (self.ptr - rhs.ptr) * Wbits;
    if(self.offset < rhs.offset)
      wdiff += (ptrdiff_t)((Wbits + self.offset) - rhs.offset) - (ptrdiff_t)Wbits;
    else
      wdiff += self.offset - rhs.offset;
    return wdiff / self.bits;
  }

  IDX operator[](const difference_type n) const {
    const Derived& self  = *static_cast<const Derived*>(this);
    return *(self + n);
  }

  // Extra methods which are not part of an iterator interface

  const W* get_ptr() const {
    const Derived& self  = *static_cast<const Derived*>(this);
    return self.ptr;
  }
  unsigned int get_offset() const {
    const Derived& self  = *static_cast<const Derived*>(this);
    return self.offset;
  }
  unsigned int get_bits() const {
    const Derived& self  = *static_cast<const Derived*>(this);
    return self.bits;
  }

  // Get some number of bits
  W get_bits(unsigned int bits) const {
    const Derived& self  = *static_cast<const Derived*>(this);
    return get<W, W>(self.ptr, bits, self.offset);
  }

  W get_bits(unsigned int bits, unsigned int offset) const {
    const Derived& self  = *static_cast<const Derived*>(this);
    return get<W, W>(self.ptr, bits, offset);
  }

  template<bool TS = false>
  void set_bits(W x, unsigned int bits) {
    Derived& self  = *static_cast<Derived*>(this);
    set<W, W, TS>(x, self.ptr, bits, self.offset);
  }
};

template<typename W, int I = sizeof(W)>
struct swap_word_mask {
  static const W value = swap_word_mask<W, I / 2>::value << (4 * I) | swap_word_mask<W, I / 2>::value;
};
template<typename W>
struct swap_word_mask<W, 1> {
  static const W value = 0x55;
};

template<typename W>
inline W swap_word(W w) {
  return ((w & swap_word_mask<W>::value) << 1) | ((w & (swap_word_mask<W>::value << 1)) >> 1);
}
template<typename W>
inline bool compare_swap_words(W w1, W w2) {
  w1 = swap_word(w1);
  w2 = swap_word(w2);
  W bmask = w1 ^ w2;
  bmask &= -bmask;
  return (w1 & bmask) == 0;
}

// Precompute (expensive) division by number of bits. The static
// arrays contain 8*sizeof(W)/k (word_idx) and k*(8*sizeof(W)/k)
// (word_bits) for k in [0, 8*sizeof(W)].

//helper template, just converts its variadic arguments to array initializer list
template<size_t... values> struct size_t_ary {static const size_t value[sizeof...(values)];};
template<size_t... values> const size_t size_t_ary<values...>::value[] = {values...};

template<typename W, int k = 8 * sizeof(W), size_t... values>
struct word_idx : word_idx <W, k-1, 8 * sizeof(W) / k, values...> {};
template<typename W,        size_t... values>
struct word_idx<W, 0, values...> : size_t_ary<(size_t)0, values...> {};

template<typename W, int k = 8 * sizeof(W), size_t... values>
struct word_bits : word_bits <W, k-1, k * (8 * sizeof(W) / k), values...> {};
template<typename W,        size_t... values>
struct word_bits<W, 0, values...> : size_t_ary<(size_t)0, values...> {};

template<typename Iterator>
bool lexicographical_compare_n(Iterator first1, const size_t len1,
                             Iterator first2, const size_t len2) {
  typedef typename Iterator::word_type W;
  const auto bits            = first1.get_bits();
  auto       left            = std::min(len1, len2) * bits;
  const decltype(len1) Widx  = word_idx<W>::value[bits];
  const decltype(len1) Wbits = word_bits<W>::value[bits];

  for( ; left > Wbits; left -= Wbits, first1 += Widx, first2 += Widx) {
    auto w1   = first1.get_bits(Wbits);
    auto w2   = first2.get_bits(Wbits);
    if(w1 != w2) return compare_swap_words(w1, w2);
  }
  if(left > 0) {
    auto w1   = first1.get_bits(left);
    auto w2   = first2.get_bits(left);
    if(w1 != w2) return compare_swap_words(w1, w2);
  }

  return len1 < len2;
}

template<typename D, typename I, typename W>
bool operator==(std::nullptr_t lfs, const common<D, I, W>& rhs) {
  return rhs == nullptr;
}

template<typename D, typename I, typename W>
D operator+(typename common<D, I, W>::difference_type lhs, const common<D, I, W>& rhs) {
  return rhs + lhs;
}

template<typename D, typename I, typename W>
std::ostream& operator<<(std::ostream& os, const common<D, I, W>& rhs) {
  return rhs.print(os);
}

template<typename IDX, typename W, bool TS = false>
class setter {
  W*           ptr;
  unsigned int bits;            // number of bits in an index
  unsigned int offset;

  typedef compact_iterator<IDX, W, TS> iterator;
public:
  setter(W* p, int b, int o) : ptr(p), bits(b), offset(o) { }
  operator IDX() const { return get<IDX, W>(ptr, bits, offset); }
  setter& operator=(const IDX x) {
    set<IDX, W, TS>(x, ptr, bits, offset);
    return *this;
  }
  setter& operator=(const setter& rhs) {
    set<IDX, W, TS>((IDX)rhs, ptr, bits, offset);
    return *this;
  }
  iterator operator&() { return iterator(ptr, bits, offset); }
};

template<typename I, typename W, bool TS>
void swap(setter<I, W, TS> x, setter<I, W, TS> y) {
  I t = x;
  x = (I)y;
  y = t;
}

} // namespace compact_iterator_imp

template<typename IDX, typename W = uint64_t, bool TS = false>
class compact_iterator :
  public std::iterator<std::random_access_iterator_tag, IDX>,
  public compact_iterator_imp::common<compact_iterator<IDX, W, TS>, IDX, W>
{
  W*           ptr;
  unsigned int bits;            // number of bits in an index
  unsigned int offset;

  friend class compact_iterator<IDX, W, !TS>;
  friend class const_compact_iterator<IDX, W>;
  friend class compact_iterator_imp::common<compact_iterator<IDX, W, TS>, IDX, W>;
  friend class compact_iterator_imp::common<const_compact_iterator<IDX, W>, IDX, W>;

  typedef compact_iterator_imp::setter<IDX, W, TS> setter_type;
  typedef std::iterator<std::random_access_iterator_tag, IDX> super;
public:
  typedef typename super::value_type      value_type;
  typedef typename super::difference_type difference_type;
  typedef IDX idx_type;
  typedef W   word_type;

  compact_iterator() = default;
  compact_iterator(W* p, unsigned int b, unsigned int o) : ptr(p), bits(b), offset(o) {
    static_assert(sizeof(IDX) <= sizeof(W), "The size of index type IDX must be less than the word type W");
  }
  template<bool TTS>
  compact_iterator(const compact_iterator<IDX, W, TTS>& rhs) : ptr(rhs.ptr), bits(rhs.bits), offset(rhs.offset) { }
  compact_iterator(std::nullptr_t) : ptr(nullptr), bits(0), offset(0) { }

  setter_type operator*() { return setter_type(ptr, bits, offset); }
  setter_type operator[](const difference_type n) const {
    return *(*this + n);
  }
};

template<typename IDX, typename W = uint64_t>
class const_compact_iterator :
  public std::iterator<std::random_access_iterator_tag, const IDX>,
  public compact_iterator_imp::common<const_compact_iterator<IDX, W>, IDX, W>
{
  const W*     ptr;
  unsigned int bits;            // number of bits in an index
  unsigned int offset;

  friend class compact_iterator<IDX, W>;
  //  friend class const_compact_iterator<IDX, W>;
  friend class compact_iterator_imp::common<compact_iterator<IDX, W>, IDX, W>;
  friend class compact_iterator_imp::common<const_compact_iterator<IDX, W>, IDX, W>;

  typedef std::iterator<std::random_access_iterator_tag, IDX> super;
public:
  typedef typename super::value_type      value_type;
  typedef typename super::difference_type difference_type;
  typedef IDX idx_type;
  typedef W   word_type;


  const_compact_iterator() = default;
  const_compact_iterator(const W* p, unsigned int b, unsigned int o) : ptr(p), bits(b), offset(o) {
    static_assert(sizeof(IDX) <= sizeof(W), "The size of index type IDX must be less than the word type W");
  }
  const_compact_iterator(const const_compact_iterator& rhs) : ptr(rhs.ptr), bits(rhs.bits), offset(rhs.offset) { }
  template<bool TS>
  const_compact_iterator(const compact_iterator<IDX, W, TS>& rhs) : ptr(rhs.ptr), bits(rhs.bits), offset(rhs.offset) { }
  const_compact_iterator(std::nullptr_t) : ptr(nullptr), bits(0), offset(0) { }
};

namespace compact_index_imp {
template<typename I, typename W>
struct const_iterator_traits<compact_iterator<I, W>> {
  typedef const_compact_iterator<I, W> type;
};
template<typename I, typename W>
struct const_iterator_traits<const_compact_iterator<I, W>> {
  typedef const_compact_iterator<I, W> type;
};

template<typename I, typename W, bool TS>
struct parallel_iterator_traits<compact_iterator<I, W, TS>> {
  typedef compact_iterator<I, W, true> type;
};

template<typename I, typename W>
struct parallel_iterator_traits<const_compact_iterator<I, W>> {
  typedef const_compact_iterator<I, W> type;
};

template<typename I, typename W>
struct prefetch_iterator_traits<compact_iterator<I, W> > {
  template<int level = 0>
  static void read(const compact_iterator<I, W>& p) { prefetch_iterator_traits<W*>::template read<level>(p.get_ptr()); }
  template<int level = 0>
  static void write(const compact_iterator<I, W>& p) { prefetch_iterator_traits<W*>::template write<level>(p.get_ptr()); }

};

template<typename I, typename W>
struct prefetch_iterator_traits<const_compact_iterator<I, W> > {
  template<int level = 0>
  static void read(const const_compact_iterator<I, W>& p) { prefetch_iterator_traits<const W*>::template read<level>(p.get_ptr()); }
  template<int level = 0>
  static void write(const const_compact_iterator<I, W>& p) { prefetch_iterator_traits<const W*>::template write<level>(p.get_ptr()); }

};
}

#endif /* __COMPACT_ITERATOR_H__ */
