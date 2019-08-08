#ifndef __TEST_COMPACT_VECTOR_H__
#define __TEST_COMPACT_VECTOR_H__

#include <gtest/gtest.h>
#include <vector>
#include <random>
#include <algorithm>
#include <thread>
#include <chrono>

#include <compact_iterator.hpp>
#include <compact_vector.hpp>
#include <misc.hpp>

namespace test_compact_vector {
extern std::mt19937_64 prg;

// Class containing the different parameters
template<typename CV>
struct TypeValueContainer {
  typedef CV                                       compact_vector_type;
  typedef typename compact_vector_type::word_type  word_type;
  typedef typename compact_vector_type::value_type vector_type;
  static constexpr bool                            thread_safe = compact_vector_type::thread_safe();
  static constexpr unsigned int                    used_bits   = compact_vector_type::used_bits();
};

template<typename CV>
void single_thread_test(size_t size, CV& vector1, CV& vector2, CV& vector3) {
  EXPECT_EQ(size, vector1.size());
  EXPECT_EQ((size_t)0, vector2.size());
  EXPECT_EQ(vector1.bits(), vector2.bits());
  EXPECT_EQ(vector1.bits(), vector3.bits());

  std::uniform_int_distribution<int> uni(0, (1 << (vector1.bits() - 1)) - 1);

  std::vector<typename CV::value_type> ary;
  {
    auto it  = vector1.begin();
    auto pit = it - 1;
    for(size_t i = 0; i < size; ++i, pit = it, ++it) {
      SCOPED_TRACE(::testing::Message() << "i:" << i);
      ary.push_back(uni(prg));
      *it = ary.back();
      vector2.push_back(ary.back());
      EXPECT_LE(vector2.size(), vector2.capacity());
      EXPECT_EQ(ary.size(), vector2.size());
      EXPECT_EQ(ary.back(), *it);
      EXPECT_EQ(ary.back(), vector1.cbegin()[i]);
      EXPECT_EQ(ary.back(), vector2.back());
      EXPECT_EQ(ary.back(), vector2.cbegin()[i]);
      EXPECT_EQ(ary.front(), vector1.front());
      EXPECT_EQ(ary.front(), vector2.front());
      EXPECT_EQ(it, &vector1.begin()[i]);
      EXPECT_EQ((ssize_t)i, it - vector1.begin());
      EXPECT_EQ(-(ssize_t)i, vector1.begin() - it);
      EXPECT_EQ(it, vector1.begin() + i);
      EXPECT_EQ(it, i + vector1.begin());
      EXPECT_EQ(vector1.begin(), it - i);
      EXPECT_EQ(pit, it - 1);
      EXPECT_EQ(it, (it - 2) + 2);
      EXPECT_TRUE(vector1.begin() <= it);
      EXPECT_TRUE(it >= vector1.begin());
    }
    EXPECT_EQ(vector1.end(), it);
  }

  {
    auto it1 = vector1.cbegin();
    auto it2 = vector2.cbegin();
    for(auto ait = ary.cbegin(); ait != ary.cend(); ++ait, ++it1, ++it2) {
      SCOPED_TRACE(::testing::Message() << "i:" << (ait - ary.cbegin()));
      EXPECT_EQ(ait - ary.cbegin(), it1 - vector1.cbegin());
      EXPECT_EQ(*ait, *it1);
      EXPECT_EQ(ait - ary.cbegin(), it2 - vector2.cbegin());
      EXPECT_EQ(*ait, *it2);
    }
  }

  {
    std::vector<size_t> order;
    auto it1 = vector1.cbegin();
    for(size_t i = 0; i < size; ++i) order.push_back(i);
    std::random_shuffle(order.begin(), order.end());
    for(auto i : order) {
      EXPECT_EQ(ary[i], it1[i]);
      EXPECT_EQ(ary[i], vector2[i]);
    }
  }

  // Test negative numbers, if vector1 is a signed type
  if(std::is_signed<typename CV::value_type>::value) {
    auto it = vector1.begin();
    for(size_t i = 0; i < size; ++i) {
      it[i] = -ary[i];
      vector3.push_back(-ary[i]);
    }
    EXPECT_EQ(ary.size(), vector3.size());
    for(size_t i = 0; i < size; ++i) {
      EXPECT_EQ(-ary[i], it[i]);
      EXPECT_EQ(-ary[i], vector3[i]);
    }
  }
}

template<typename CV>
void set_get_all(CV& vector1) {
  typedef typename CV::value_type value_type;

  const value_type max = 1 << vector1.bits();
  const value_type low = std::is_signed<value_type>::value ? -(max/2) : 0;
  const value_type high = std::is_signed<value_type>::value ? max/2 - 1 : max - 1;
  for(size_t i = 0; i < vector1.size(); ++i) {
    SCOPED_TRACE(::testing::Message() << "i:" << i);
    for(value_type j = low; j <= high; ++j) {
      vector1[i] = j;
      ASSERT_EQ(j, vector1.begin()[i]);
      ASSERT_EQ(j, vector1.cbegin()[i]);
    }
  }
}

} // namespace test_compact_vector

#endif /* __TEST_COMPACT_VECTOR_H__ */
