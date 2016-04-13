#ifndef __PARALLEL_POINTER_TRAITS_H__
#define __PARALLEL_POINTER_TRAITS_H__

#include <type_traits>

namespace compact_index_imp {
// Traits for a parallel iterator. Very weak requirements that if two
// threads hold iterators to two different location, then the pointers
// can be read and stored.
//
// This holds for pointers. But it requires attention when dealing
// with compact iterators.

template<typename T> struct parallel_iterator_traits { };

template<typename T>
struct parallel_iterator_traits<T*> {
  typedef T* type;
};

template<typename T>
struct parallel_iterator_traits<const T*> {
  typedef const T* type;
};
} // namespace compact_index_imp

#endif /* __PARALLEL_POINTER_TRAITS_H__ */
