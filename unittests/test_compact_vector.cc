#include <gtest/gtest.h>
#include <vector>
#include <random>
#include <algorithm>
#include <thread>
#include <chrono>

#include <compact_iterator.hpp>
#include <compact_vector.hpp>

namespace {
TEST(CompactVector, RequiredBits) {
  for(unsigned int i = 1; i < sizeof(size_t) * 8 - 1; ++i) {
    size_t s = (size_t)1 << i;
    EXPECT_EQ(i,     compact::vector<uint64_t>::required_bits(s));
    EXPECT_EQ(i + 1, compact::vector<int64_t>::required_bits(s));
    EXPECT_EQ(i + 1, compact::vector<uint64_t>::required_bits(s + 1));
    EXPECT_EQ(i + 2, compact::vector<int64_t>::required_bits(s + 1));
  }
} // CompactVector.RequiredBits

//
// Testing compact::vector_imp::vector for different vector type, word type, bits and used bits value.
//

// Class containing the different parameters
template<typename CV>
struct TypeValueContainer {
  typedef CV                                       compact_vector_type;
  typedef typename compact_vector_type::word_type  word_type;
  typedef typename compact_vector_type::value_type vector_type;
  static constexpr bool                            thread_safe = compact_vector_type::thread_safe();
  static constexpr unsigned int                    used_bits   = compact_vector_type::used_bits();
};

// Type parameterized test
template<typename T>
class CompactVectorDynTest : public ::testing::Test {
protected:
  static constexpr int bits[8] = {1, 2, 3, 4, 5, 6, 8, 13};
  static constexpr size_t size = 1000;
};
template<typename T>
constexpr int CompactVectorDynTest<T>::bits[8];
// template<typename T>
// const size_t CompactVectorDynTest<T>::size;
TYPED_TEST_CASE_P(CompactVectorDynTest);

template<typename CV>
void single_thread_test(size_t size, CV& vector1, CV& vector2, CV& vector3) {
  EXPECT_EQ(size, vector1.size());
  EXPECT_EQ((size_t)0, vector2.size());
  EXPECT_EQ(vector1.bits(), vector2.bits());
  EXPECT_EQ(vector1.bits(), vector3.bits());

  std::default_random_engine         generator;
  std::uniform_int_distribution<int> uni(0, (1 << (vector1.bits() - 1)) - 1);

  std::vector<typename CV::value_type> ary;
  {
    auto it  = vector1.begin();
    auto pit = it - 1;
    for(size_t i = 0; i < size; ++i, pit = it, ++it) {
      SCOPED_TRACE(::testing::Message() << "i:" << i);
      ary.push_back(uni(generator));
      *it = ary.back();
      vector2.push_back(ary.back());
      EXPECT_LE(vector2.size(), vector2.capacity());
      EXPECT_LE(vector2.capacity(), 2*vector2.size());
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

TYPED_TEST_P(CompactVectorDynTest, DynIterator) {
  for(size_t i = 0; i < sizeof(this->bits) / sizeof(int); ++i) {
    const int bits = this->bits[i];
    SCOPED_TRACE(::testing::Message() << "Dynamic bits:" << bits);
    typename TypeParam::compact_vector_type vector1(bits, this->size);
    typename TypeParam::compact_vector_type vector2(bits);
    typename TypeParam::compact_vector_type vector3(bits);

    EXPECT_EQ((unsigned int)bits, vector1.bits());
    EXPECT_EQ((unsigned int)bits, vector2.bits());
    EXPECT_EQ((unsigned int)bits, vector3.bits());

    single_thread_test(this->size, vector1, vector2, vector3);
  }
}

TYPED_TEST_P(CompactVectorDynTest, DynSwap) {
  std::default_random_engine         generator;

  for(size_t i = 0; i < sizeof(this->bits) / sizeof(int); ++i) {
    const int                              bits = this->bits[i];
    SCOPED_TRACE(::testing::Message() << "bits:" << bits);
    typename TypeParam::compact_vector_type vector(bits, this->size);
    std::uniform_int_distribution<int>     uni(0, (1 << (bits - 1)) - 1);
    const typename TypeParam::vector_type   v1   = uni(generator);
    const typename TypeParam::vector_type   v2   = uni(generator);

    auto it = vector.begin();
    auto jt = it + 10;
    *it = v1;
    *jt = v2;

    EXPECT_EQ(v1, *it);
    EXPECT_EQ(v2, *jt);
    swap(*it, *jt);
    EXPECT_EQ(v2, *it);
    EXPECT_EQ(v1, *jt);

    *it = *jt;
    EXPECT_EQ(*it, *jt);
    EXPECT_EQ(v1, *it);
    EXPECT_EQ(v1, *jt);
  } // for(). Loop over this->bits
} // CompactIterator.Swap

// Instantiate the typed tests for dynamic bits
REGISTER_TYPED_TEST_CASE_P(CompactVectorDynTest, DynIterator, DynSwap);
typedef ::testing::Types<TypeValueContainer<compact::vector<int>>,
                         TypeValueContainer<compact::vector<unsigned>>,

                         // Same thing with uint32 word type
                         TypeValueContainer<compact::vector<int, 0, uint32_t>>,
                         TypeValueContainer<compact::vector<unsigned, 0, uint32_t>>,

                         // Same tests with other vector types (with different thread safety)
                         TypeValueContainer<compact::ts_vector<int>>,
                         TypeValueContainer<compact::ts_vector<unsigned>>,
                         TypeValueContainer<compact::ts_vector<int, 0, uint32_t>>,
                         TypeValueContainer<compact::ts_vector<unsigned, 0, uint32_t>>,

                         TypeValueContainer<compact::cas_vector<int>>,
                         TypeValueContainer<compact::cas_vector<unsigned>>,
                         TypeValueContainer<compact::cas_vector<int, 0, uint32_t>>,
                         TypeValueContainer<compact::cas_vector<unsigned, 0, uint32_t>>
                         > compact_vector_types;
INSTANTIATE_TYPED_TEST_CASE_P(CompactVectorDyn, CompactVectorDynTest, compact_vector_types);

// Same test but with static bits usage
template<typename T>
class CompactVectorStatTest : public ::testing::Test {
protected:
  static constexpr size_t size = 1000;
};
TYPED_TEST_CASE_P(CompactVectorStatTest);
TYPED_TEST_P(CompactVectorStatTest, StatIterator) {
  typedef typename TypeParam::compact_vector_type compact_vector_type;
  SCOPED_TRACE(::testing::Message() << "Static bits:" << compact_vector_type::static_bits());
  compact_vector_type vector1(this->size);
  compact_vector_type vector2, vector3;

  EXPECT_EQ(vector1.static_bits(), vector1.bits());
  EXPECT_EQ(vector2.static_bits(), vector2.bits());
  EXPECT_EQ(vector3.static_bits(), vector3.bits());

  single_thread_test(this->size, vector1, vector2, vector3);
}
REGISTER_TYPED_TEST_CASE_P(CompactVectorStatTest, StatIterator);
typedef ::testing::Types<TypeValueContainer<compact::vector<int, 1>>,
                         TypeValueContainer<compact::vector<int, 2>>,
                         TypeValueContainer<compact::vector<int, 3>>,
                         TypeValueContainer<compact::vector<int, 4>>,
                         TypeValueContainer<compact::vector<int, 5>>,
                         TypeValueContainer<compact::vector<unsigned, 1>>,
                         TypeValueContainer<compact::vector<unsigned, 2>>,
                         TypeValueContainer<compact::vector<unsigned, 3>>,
                         TypeValueContainer<compact::vector<unsigned, 4>>,
                         TypeValueContainer<compact::vector<unsigned, 5>>,
                         TypeValueContainer<compact::ts_vector<int, 1>>,
                         TypeValueContainer<compact::ts_vector<int, 2>>,
                         TypeValueContainer<compact::ts_vector<int, 3>>,
                         TypeValueContainer<compact::ts_vector<int, 4>>,
                         TypeValueContainer<compact::ts_vector<int, 5>>,
                         TypeValueContainer<compact::ts_vector<unsigned, 1>>,
                         TypeValueContainer<compact::ts_vector<unsigned, 2>>,
                         TypeValueContainer<compact::ts_vector<unsigned, 3>>,
                         TypeValueContainer<compact::ts_vector<unsigned, 4>>,
                         TypeValueContainer<compact::ts_vector<unsigned, 5>>,
                         TypeValueContainer<compact::cas_vector<int, 1>>,
                         TypeValueContainer<compact::cas_vector<int, 2>>,
                         TypeValueContainer<compact::cas_vector<int, 3>>,
                         TypeValueContainer<compact::cas_vector<int, 4>>,
                         TypeValueContainer<compact::cas_vector<int, 5>>,
                         TypeValueContainer<compact::cas_vector<unsigned, 1>>,
                         TypeValueContainer<compact::cas_vector<unsigned, 2>>,
                         TypeValueContainer<compact::cas_vector<unsigned, 3>>,
                         TypeValueContainer<compact::cas_vector<unsigned, 4>>,
                         TypeValueContainer<compact::cas_vector<unsigned, 5>>
                         > compact_vector_stat_types;
INSTANTIATE_TYPED_TEST_CASE_P(CompactVectorStat, CompactVectorStatTest, compact_vector_stat_types);


TEST(CompactIterator, Nullptr) {
  compact::iterator<int> it = nullptr;

  EXPECT_EQ(nullptr, it);
} // CompactVector.Pointer

void set_values(int thid, int nb_threads, compact::vector<int>::iterator ary, size_t size) {
  typedef compact::parallel_iterator_traits<compact::vector<int>::iterator>::type pary_type;
  pary_type pary(ary);

  for(int i = 0; i < 1000; ++i)
    for(size_t j = thid; j < size; j += nb_threads)
      pary[j] = i + 1;
}

TEST(CompactVector2, MultiThread) {
  const unsigned int   bits       = 13;
  const size_t         size       = 64;
  const int            nb_threads = 4;
  compact::vector<int> vector1(bits, size);

  std::fill_n(vector1.begin(), size, 0);
  std::vector<std::thread> threads;
  for(int i = 0; i < nb_threads; ++i)
    threads.push_back(std::thread(set_values, i, nb_threads, vector1.begin(), size));
  for(int i = 0; i < nb_threads; ++i)
    threads[i].join();

  for(size_t i = 0; i < size; ++i)
    EXPECT_EQ(1000, vector1[i]);
} // CompactVector.MultiThread

template<typename Iterator>
void cas_values(int val, Iterator ary, size_t size, size_t* nb_success) {
  // Try to set my thid in every location. Succeed only if already zero
  std::default_random_engine         gen;
  std::uniform_int_distribution<int> dist(10, 100);

  for(size_t i = 0; i < size; ++i, ++ary) {
    unsigned int expected = 0;
    *nb_success += compact::parallel_iterator_traits<Iterator>::cas(ary, expected, val);
    // if(*ary != val) {
    //   std::cerr << i << ' ' << *ary << ' ' << val << std::endl;
    //   asm("int3");
    // }
      //    assert(*ary == val);
    if(i % 128 == 0)
      std::this_thread::sleep_for(std::chrono::microseconds(dist(gen)));
  }
}

TEST(CompactVector2, CAS) {
  const size_t size       = 1024 * 1024;
  const int    nb_threads = 4;
  const int    bits       = 3;

  std::vector<unsigned int> ptr(size, 0);
  typedef compact::cas_vector<unsigned int> compact_vector_type;
  typedef compact_vector_type::iterator compact_iterator_type;
  compact_vector_type vector1(bits, size);

  std::vector<std::thread> threads;

  // Cas values on int*
  std::vector<size_t> successes_ptr(nb_threads, 0);
  for(int i = 0; i < nb_threads; ++i) {
    threads.push_back(std::thread(cas_values<unsigned int*>, i + 1, ptr.data(), size, &successes_ptr[i]));
  }
  for(int i = 0; i < nb_threads; ++i)
    threads[i].join();

  threads.clear();
  // CAS values on compact iterator
  std::vector<size_t> successes_ci(nb_threads, 0);
  for(int i = 0; i < nb_threads; ++i) {
    threads.push_back(std::thread(cas_values<compact_iterator_type>, i + 1, vector1.mt_begin(), size, &successes_ci[i]));
  }
  for(int i = 0; i < nb_threads; ++i)
    threads[i].join();

  size_t total_successes_ptr = 0;
  for(const auto s : successes_ptr) {
    total_successes_ptr += s;
  }
  size_t total_successes_ci  = 0;
  for(const auto s : successes_ci) {
    total_successes_ci += s;
  }

  EXPECT_EQ(size, total_successes_ptr);
  EXPECT_EQ(size, total_successes_ci);
  for(const auto v : ptr) {
    EXPECT_TRUE(v >= 1 && v <= nb_threads);
    --successes_ptr[v - 1];
  }
  for(auto it = vector1.cbegin(); it != vector1.cend(); ++it) {
    SCOPED_TRACE(::testing::Message() << "i:" << (it - vector1.cbegin()));
    ASSERT_LE((unsigned int)1, *it);
    ASSERT_GE((unsigned int)nb_threads, *it);
    --successes_ci[*it - 1];
  }

  for(const auto s : successes_ptr)
    EXPECT_EQ((size_t)0, s);
  for(const auto s : successes_ci)
    EXPECT_EQ((size_t)0, s);
} // CompactVector.CAS

} // empty namespace
