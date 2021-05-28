#include <unittests/test_compact_vector.hpp>

#include "allocator_fill_random.hpp"


namespace test_compact_vector {
// using test_compact_vector::prg;

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
TYPED_TEST_SUITE_P(CompactVectorDynTest);

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
  for(size_t i = 0; i < sizeof(this->bits) / sizeof(int); ++i) {
    const int                              bits = this->bits[i];
    SCOPED_TRACE(::testing::Message() << "bits:" << bits);
    typename TypeParam::compact_vector_type vector(bits, this->size);
    std::uniform_int_distribution<int>     uni(0, (1 << (bits - 1)) - 1);
    const typename TypeParam::vector_type   v1   = uni(prg);
    const typename TypeParam::vector_type   v2   = uni(prg);

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
template<typename T, typename W> using vector_type = compact::vector<T, 0, W, allocator_fill_random<W> >;
template<typename T, typename W> using ts_vector_type = compact::ts_vector<T, 0, W, allocator_fill_random<W> >;
template<typename T, typename W> using cas_vector_type = compact::cas_vector<T, 0, W, allocator_fill_random<W> >;

REGISTER_TYPED_TEST_SUITE_P(CompactVectorDynTest, DynIterator, DynSwap);
typedef ::testing::Types<TypeValueContainer<vector_type<int, uint64_t>>,
                         TypeValueContainer<vector_type<unsigned, uint64_t>>,

                         // Same thing with uint32 word type
                         TypeValueContainer<vector_type<int, uint32_t>>,
                         TypeValueContainer<vector_type<unsigned, uint32_t>>,

                         // Same tests with other vector types (with different thread safety)
                         TypeValueContainer<ts_vector_type<int, uint64_t>>,
                         TypeValueContainer<ts_vector_type<unsigned, uint64_t>>,
                         TypeValueContainer<ts_vector_type<int, uint32_t>>,
                         TypeValueContainer<ts_vector_type<unsigned, uint32_t>>,

                         TypeValueContainer<cas_vector_type<int, uint64_t>>,
                         TypeValueContainer<cas_vector_type<unsigned, uint64_t>>,
                         TypeValueContainer<cas_vector_type<int, uint32_t>>,
                         TypeValueContainer<cas_vector_type<unsigned, uint32_t>>
                         > compact_vector_types;
INSTANTIATE_TYPED_TEST_SUITE_P(CompactVectorDyn, CompactVectorDynTest, compact_vector_types);
} // namespace
