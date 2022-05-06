#ifndef __ALLOCATOR_FILL_RANDOM_H__
#define __ALLOCATOR_FILL_RANDOM_H__

#include <fstream>
#include <iterator>

// Standard allocator, but fill the allocated region with random bits

template <typename T>
class allocator_fill_random : public std::allocator<T> {
  typedef std::allocator<T> super;
//  typedef std::allocator_traits<allocator_fill_random<T>> traits;

public:
  typedef typename super::value_type                             value_type;
  typedef value_type*                                            pointer;
  typedef value_type&                                            reference;
  typedef const value_type*                                      const_pointer;
  typedef const value_type&                                      const_reference;
  typedef typename super::size_type                              size_type;
  typedef typename super::difference_type                        difference_type;
  //  typedef typename super::rebind                                 rebind;
  typedef typename super::propagate_on_container_move_assignment propagate_on_container_move_assignment;

  allocator_fill_random() noexcept
  : super()
  { }

  allocator_fill_random (const allocator_fill_random& alloc) noexcept
    : super(alloc)
  { }

  template <class U>
  allocator_fill_random (const allocator_fill_random<U>& alloc) noexcept
    : super(alloc)
  { }

  pointer allocate(size_type n, const_pointer hint=0) {
    auto res = super::allocate(n);

    //    std::fill_n(res, n, (T)-1);
    std::ifstream rand("/dev/urandom");
    rand.read((char*)res, sizeof(T) * n);
    if(!rand.good()) throw std::runtime_error("Can't open /dev/urandom");
    return res;
  }
};

#endif /* __ALLOCATOR_FILL_RANDOM_H__ */
