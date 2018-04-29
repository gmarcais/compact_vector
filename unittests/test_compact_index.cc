#include <gtest/gtest.h>
#include <vector>
#include <random>
#include <algorithm>
#include <thread>
#include <chrono>

#include <compact_iterator.hpp>
#include <compact_index.hpp>

namespace {
TEST(CompactIndex, RequiredBits) {
  for(unsigned int i = 1; i < sizeof(size_t) * 8 - 1; ++i) {
    size_t s = (size_t)1 << i;
    EXPECT_EQ(i,     compact::vector<uint64_t>::required_bits(s));
    EXPECT_EQ(i + 1, compact::vector<int64_t>::required_bits(s));
    EXPECT_EQ(i + 1, compact::vector<uint64_t>::required_bits(s + 1));
    EXPECT_EQ(i + 2, compact::vector<int64_t>::required_bits(s + 1));
  }
} // CompactIndex.RequiredBits

//
// Testing compact::index_imp::index for different index type, word type, bits and used bits value.
//

// Class containing the different parameters
template<typename IDX, typename W, bool TS, unsigned int UB>
struct TypeValueContainer {
  typedef IDX                                                          index_type;
  typedef W                                                            word_type;
  typedef compact::index_imp::index<IDX, W, std::allocator<W>, UB, TS> compact_index_type;
  static const bool                                                    thread_safe = TS;
  static const unsigned int                                            used_bits   = UB;
};
template<typename IDX, typename W, bool TS, unsigned int UB>
const bool TypeValueContainer<IDX,W,TS,UB>::thread_safe;
template<typename IDX, typename W, bool TS, unsigned int UB>
const unsigned int TypeValueContainer<IDX,W,TS,UB>::used_bits;

// Type parameterized test
template<typename T>
class CompactIndexTest : public ::testing::Test {
protected:
  static constexpr int bits[4] = {5, 6, 8, 13};
  static const size_t  size    = 1000;
};
template<typename T>
constexpr int CompactIndexTest<T>::bits[4];
template<typename T>
const size_t CompactIndexTest<T>::size;
TYPED_TEST_CASE_P(CompactIndexTest);

TYPED_TEST_P(CompactIndexTest, Iterator) {
  std::default_random_engine         generator;
  for(size_t i = 0; i < sizeof(this->bits) / sizeof(int); ++i) {
    const int bits = this->bits[i];
    SCOPED_TRACE(::testing::Message() << "bits:" << bits);
    typename TypeParam::compact_index_type index(this->size, bits);

    EXPECT_EQ(this->size, index.size());
    EXPECT_EQ((unsigned int)bits, index.bits());

    std::uniform_int_distribution<int> uni(0, (1 << (bits - 1)) - 1);

    std::vector<typename TypeParam::index_type> ary;
    {
      auto it  = index.begin();
      auto pit = it - 1;
      for(size_t i = 0; i < this->size; ++i, pit = it, ++it) {
        SCOPED_TRACE(::testing::Message() << "i:" << i);
        ary.push_back(uni(generator));
        *it = ary.back();
        EXPECT_EQ(ary.back(), *it);
        EXPECT_EQ(ary.back(), index.cbegin()[i]);
        EXPECT_EQ(it, &index.begin()[i]);
        EXPECT_EQ((ssize_t)i, it - index.begin());
        EXPECT_EQ(-(ssize_t)i, index.begin() - it);
        EXPECT_EQ(it, index.begin() + i);
        EXPECT_EQ(it, i + index.begin());
        EXPECT_EQ(index.begin(), it - i);
        EXPECT_EQ(pit, it - 1);
        EXPECT_EQ(it, (it - 2) + 2);
        EXPECT_TRUE(index.begin() <= it);
        EXPECT_TRUE(it >= index.begin());
      }
      EXPECT_EQ(index.end(), it);
    }

    {
      auto it = index.cbegin();
      for(auto ait = ary.cbegin(); ait != ary.cend(); ++ait, ++it) {
        SCOPED_TRACE(::testing::Message() << "i:" << (ait - ary.cbegin()));
        EXPECT_EQ(ait - ary.cbegin(), it - index.cbegin());
        EXPECT_EQ(*ait, *it);
      }
    }

    {
      std::vector<size_t> order;
      auto it = index.cbegin();
      for(size_t i = 0; i < this->size; ++i) order.push_back(i);
      std::random_shuffle(order.begin(), order.end());
      for(auto i : order)
        EXPECT_EQ(ary[i], it[i]);
    }

    // Test negative numbers, if index is a signed type
    if(std::is_signed<typename TypeParam::index_type>::value) {
      auto it = index.begin();
      for(size_t i = 0; i < this->size; ++i)
        it[i] = -ary[i];
      for(size_t i = 0; i < this->size; ++i)
        EXPECT_EQ(-ary[i], it[i]);
    }
  } // for(). Loop over this->bits
}

TYPED_TEST_P(CompactIndexTest, Swap) {
  std::default_random_engine         generator;

  for(size_t i = 0; i < sizeof(this->bits) / sizeof(int); ++i) {
    const int                              bits = this->bits[i];
    SCOPED_TRACE(::testing::Message() << "bits:" << bits);
    typename TypeParam::compact_index_type index(this->size, bits);
    std::uniform_int_distribution<int>     uni(0, (1 << (bits - 1)) - 1);
    const typename TypeParam::index_type   v1   = uni(generator);
    const typename TypeParam::index_type   v2   = uni(generator);

    auto it = index.begin();
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

// Instantiate the typed tests with too many values
REGISTER_TYPED_TEST_CASE_P(CompactIndexTest, Iterator, Swap);
typedef ::testing::Types<TypeValueContainer<int, uint64_t, false, compact::bitsof<uint64_t>::val>,
                         TypeValueContainer<unsigned int, uint64_t, false, compact::bitsof<uint64_t>::val>,

                         // Same thing with uint32 word type
                         TypeValueContainer<int, uint32_t, false, compact::bitsof<uint32_t>::val>,
                         TypeValueContainer<unsigned int, uint32_t, false, compact::bitsof<uint32_t>::val>,

                         // Same tests, but don't use every bits in the words
                         TypeValueContainer<int, uint64_t, false, compact::bitsof<uint64_t>::val - 1>,
                         TypeValueContainer<unsigned int, uint64_t, false, compact::bitsof<uint64_t>::val - 1>,
                         TypeValueContainer<int, uint32_t, false, compact::bitsof<uint32_t>::val - 1>,
                         TypeValueContainer<unsigned int, uint32_t, false, compact::bitsof<uint32_t>::val - 1>
                         > compact_index_types;
INSTANTIATE_TYPED_TEST_CASE_P(CompactIndex, CompactIndexTest, compact_index_types);

TEST(CompactIterator, Nullptr) {
  compact::iterator<int> it = nullptr;

  EXPECT_EQ(nullptr, it);
} // CompactIndex.Pointer

void set_values(int thid, int nb_threads, compact::vector<int>::iterator ary, size_t size) {
  typedef compact::index_imp::parallel_iterator_traits<compact::vector<int>::iterator>::type pary_type;
  pary_type pary(ary);

  for(int i = 0; i < 1000; ++i)
    for(size_t j = thid; j < size; j += nb_threads)
      pary[j] = i + 1;
}

TEST(CompactIndex2, MultiThread) {
  const unsigned int   bits       = 13;
  const size_t         size       = 64;
  const int            nb_threads = 4;
  compact::vector<int> index(size, bits);

  std::fill_n(index.begin(), size, 0);
  std::vector<std::thread> threads;
  for(int i = 0; i < nb_threads; ++i)
    threads.push_back(std::thread(set_values, i, nb_threads, index.begin(), size));
  for(int i = 0; i < nb_threads; ++i)
    threads[i].join();

  for(size_t i = 0; i < size; ++i)
    EXPECT_EQ(1000, index[i]);
} // CompactIndex.MultiThread

template<typename Iterator>
void cas_values(int val, Iterator ary, size_t size, size_t* nb_success) {
  // Try to set my thid in every location. Succeed only if already zero
  std::default_random_engine         gen;
  std::uniform_int_distribution<int> dist(10, 100);

  for(size_t i = 0; i < size; ++i, ++ary) {
    unsigned int expected = 0;
    *nb_success += compact::index_imp::parallel_iterator_traits<Iterator>::cas(ary, expected, val);
    // if(*ary != val) {
    //   std::cerr << i << ' ' << *ary << ' ' << val << std::endl;
    //   asm("int3");
    // }
      //    assert(*ary == val);
    if(i % 128 == 0)
      std::this_thread::sleep_for(std::chrono::microseconds(dist(gen)));
  }
}

TEST(CompactIndex2, CAS) {
  const size_t size       = 1024 * 1024;
  const int    nb_threads = 4;
  const int    bits       = 3;

  std::vector<unsigned int> ptr(size, 0);
  typedef compact::cas_vector<unsigned int> compact_index_type;
  typedef compact_index_type::iterator compact_iterator_type;
  compact_index_type index(size, bits);

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
    threads.push_back(std::thread(cas_values<compact_iterator_type>, i + 1, index.mt_begin(), size, &successes_ci[i]));
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
  for(auto it = index.cbegin(); it != index.cend(); ++it) {
    SCOPED_TRACE(::testing::Message() << "i:" << (it - index.cbegin()));
    ASSERT_LE((unsigned int)1, *it);
    ASSERT_GE((unsigned int)nb_threads, *it);
    --successes_ci[*it - 1];
  }

  for(const auto s : successes_ptr)
    EXPECT_EQ((size_t)0, s);
  for(const auto s : successes_ci)
    EXPECT_EQ((size_t)0, s);
} // CompactIndex.CAS

} // empty namespace
