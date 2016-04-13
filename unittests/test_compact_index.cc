#include <gtest/gtest.h>
#include <vector>
#include <random>
#include <algorithm>
#include <thread>

#include <compact_iterator.hpp>
#include <compact_index.hpp>

namespace {
TEST(CompactIndex, RequiredBits) {
  for(unsigned int i = 1; i < sizeof(size_t) * 8 - 1; ++i) {
    size_t s = (size_t)1 << i;
    EXPECT_EQ(i,     compact_index<uint64_t>::required_bits(s));
    EXPECT_EQ(i + 1, compact_index<int64_t>::required_bits(s));
    EXPECT_EQ(i + 1, compact_index<uint64_t>::required_bits(s + 1));
    EXPECT_EQ(i + 2, compact_index<int64_t>::required_bits(s + 1));
  }
} // CompactIndex.RequiredBits

TEST(CompactIndex, Iterator) {
  static const size_t       size = 1000;
  static const unsigned int bits = 13;
  compact_index<int>        index(size, bits);

  EXPECT_EQ(size, index.size);
  EXPECT_EQ(bits, index.bits);

  std::default_random_engine         generator;
  std::uniform_int_distribution<int> uni(0, (1 << (bits - 1)) - 1);

  std::vector<int> ary;
  {
    auto it  = index.begin();
    auto pit = it - 1;
    for(size_t i = 0; i < size; ++i, pit = it, ++it) {
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
    for(size_t i = 0; i < size; ++i) order.push_back(i);
    std::random_shuffle(order.begin(), order.end());
    for(auto i : order)
      EXPECT_EQ(ary[i], it[i]);
  }

  // Test negative numbers
  {
    auto it = index.begin();
    for(size_t i = 0; i < size; ++i)
      it[i] = -ary[i];
    for(size_t i = 0; i < size; ++i)
      EXPECT_EQ(-ary[i], it[i]);
  }
}

TEST(CompactIndex, Nullptr) {
  compact_iterator<int> it = nullptr;

  EXPECT_EQ(nullptr, it);
} // CompactIndex.Pointer

TEST(CompactIndex, Swap) {
  static const size_t       size = 1000;
  static const unsigned int bits = 13;
  const int                 v1   = 123;
  const int                 v2   = 456;
  compact_index<int>        index(size, bits);

  auto it = index.begin();
  auto jt = it + 10;
  *it = v1;
  *jt = v2;

  EXPECT_NE(*it, *jt);
  swap(*it, *jt);
  EXPECT_EQ(v2, *it);
  EXPECT_EQ(v1, *jt);

  *it = *jt;
  EXPECT_EQ(*it, *jt);
  EXPECT_EQ(v1, *it);
  EXPECT_EQ(v1, *jt);
} // CompactIndex.Swap

void set_values(int thid, int nb_threads, compact_index<int>::iterator ary, size_t size) {
  typedef compact_index_imp::parallel_iterator_traits<compact_index<int>::iterator>::type pary_type;
  pary_type pary(ary);

  for(int i = 0; i < 1000; ++i)
    for(size_t j = thid; j < size; j += nb_threads)
      pary[j] = i + 1;
}

TEST(CompactIndex, MultiThread) {
  const unsigned int bits       = 13;
  const size_t       size       = 64;
  const int          nb_threads = 4;
  compact_index<int> index(size, bits);

  std::fill_n(index.begin(), size, 0);
  std::vector<std::thread> threads;
  for(int i = 0; i < nb_threads; ++i)
    threads.push_back(std::thread(set_values, i, nb_threads, index.begin(), size));
  for(int i = 0; i < nb_threads; ++i)
    threads[i].join();

  for(size_t i = 0; i < size; ++i)
    EXPECT_EQ(1000, index[i]);
} // CompactIndex.MultiThread
} // empty namespace
