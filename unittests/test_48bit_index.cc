#include <gtest/gtest.h>
#include <random>
#include <vector>
#include <utility>

#include <48bit_index.hpp>

namespace {
using std::swap;
TEST(FortyEightIndex, Iterator) {
  static const size_t size = 1000;
  fortyeight_index<int64_t> index_signed(size);
  fortyeight_index<uint64_t> index_unsigned(size);

  std::default_random_engine              generator;
  std::uniform_int_distribution<int64_t>  uni_signed(-((int64_t)1 << 46), ((int64_t)1 << 46) - 1);
  std::uniform_int_distribution<uint64_t> uni_unsigned((uint64_t)0, ((uint64_t)1 << 47) - 1);

  std::vector<int64_t> ary_signed;
  std::vector<uint64_t> ary_unsigned;
  int64_t val;
  uint64_t unval;
  for(int loop = 0; loop < 2; ++loop) { // First loop set. Second loop only get.
    auto it_signed = index_signed.begin();
    auto it_unsigned = index_unsigned.begin();
    for(size_t i = 0; i < size; ++i, ++it_signed, ++it_unsigned) {
      SCOPED_TRACE(::testing::Message() << "i:" << i);
      EXPECT_NE(index_signed.end(), it_signed);
      EXPECT_NE(index_unsigned.end(), it_unsigned);

      if(loop == 0) {
        ary_signed.push_back(uni_signed(generator));
        val = *it_signed = ary_signed.back();
        ary_unsigned.push_back(uni_unsigned(generator));
        unval = *it_unsigned = ary_unsigned.back();
      } else {
        val = ary_signed[i];
        unval = ary_unsigned[i];
      }

      EXPECT_EQ(val, *it_signed);
      EXPECT_EQ(val, index_signed[i]);
      EXPECT_EQ(unval, *it_unsigned);
      EXPECT_EQ(unval, index_unsigned[i]);

      EXPECT_EQ((ssize_t)i, (ssize_t)(it_signed - index_signed.begin()));
      EXPECT_EQ((ssize_t)i, (ssize_t)(it_unsigned - index_unsigned.begin()));
      EXPECT_EQ(index_signed.begin() + i, it_signed);
      EXPECT_EQ(index_unsigned.begin() + i, it_unsigned);
      EXPECT_EQ(index_signed.end() - (size - i), it_signed);
      EXPECT_EQ(index_unsigned.end() - (size - i), it_unsigned);
    }
    EXPECT_EQ(index_signed.end(), it_signed);
    EXPECT_EQ(index_unsigned.end(), it_unsigned);
  } // loop

  // Random swap
  std::uniform_int_distribution<size_t> rand_pos(0, size - 1);
  for(int i = 0; i < 1000; ++i) {
    size_t x = rand_pos(generator);
    size_t y = rand_pos(generator);

    swap(index_signed[x], index_signed[y]);
    swap(ary_signed[x], ary_signed[y]);
    EXPECT_EQ(ary_signed[x], index_signed[x]);
    EXPECT_EQ(ary_signed[y], index_signed[y]);

    swap(index_unsigned[x], index_unsigned[y]);
    swap(ary_unsigned[x], ary_unsigned[y]);
    EXPECT_EQ(ary_unsigned[x], index_unsigned[x]);
    EXPECT_EQ(ary_unsigned[y], index_unsigned[y]);
}
} // FortyEightIndex.Iterator

} // namespac
