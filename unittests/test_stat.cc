#include <unittests/test_compact_vector.hpp>

#include "allocator_fill_random.hpp"

namespace test_compact_vector {
//using test_compact_vector::prg;

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
  set_get_all(vector1);
}

template<typename T, int B> using vector_type = compact::vector<T, B, uint64_t, allocator_fill_random<uint64_t>>;
template<typename T, int B> using ts_vector_type = compact::ts_vector<T, B, uint64_t, allocator_fill_random<uint64_t>>;
template<typename T, int B> using cas_vector_type = compact::cas_vector<T, B, uint64_t, allocator_fill_random<uint64_t>>;

REGISTER_TYPED_TEST_CASE_P(CompactVectorStatTest, StatIterator);
typedef ::testing::Types<TypeValueContainer<vector_type<int, 1>>,
                         TypeValueContainer<vector_type<int, 2>>,
                         TypeValueContainer<vector_type<int, 3>>,
                         TypeValueContainer<vector_type<int, 4>>,
                         TypeValueContainer<vector_type<int, 5>>,
                         TypeValueContainer<vector_type<unsigned, 1>>,
                         TypeValueContainer<vector_type<unsigned, 2>>,
                         TypeValueContainer<vector_type<unsigned, 3>>,
                         TypeValueContainer<vector_type<unsigned, 4>>,
                         TypeValueContainer<vector_type<unsigned, 5>>,
                         TypeValueContainer<ts_vector_type<int, 1>>,
                         TypeValueContainer<ts_vector_type<int, 2>>,
                         TypeValueContainer<ts_vector_type<int, 3>>,
                         TypeValueContainer<ts_vector_type<int, 4>>,
                         TypeValueContainer<ts_vector_type<int, 5>>,
                         TypeValueContainer<ts_vector_type<unsigned, 1>>,
                         TypeValueContainer<ts_vector_type<unsigned, 2>>,
                         TypeValueContainer<ts_vector_type<unsigned, 3>>,
                         TypeValueContainer<ts_vector_type<unsigned, 4>>,
                         TypeValueContainer<ts_vector_type<unsigned, 5>>,
                         TypeValueContainer<cas_vector_type<int, 1>>,
                         TypeValueContainer<cas_vector_type<int, 2>>,
                         TypeValueContainer<cas_vector_type<int, 3>>,
                         TypeValueContainer<cas_vector_type<int, 4>>,
                         TypeValueContainer<cas_vector_type<int, 5>>,
                         TypeValueContainer<cas_vector_type<unsigned, 1>>,
                         TypeValueContainer<cas_vector_type<unsigned, 2>>,
                         TypeValueContainer<cas_vector_type<unsigned, 3>>,
                         TypeValueContainer<cas_vector_type<unsigned, 4>>,
                         TypeValueContainer<cas_vector_type<unsigned, 5>>
                         > compact_vector_stat_types;
INSTANTIATE_TYPED_TEST_CASE_P(CompactVectorStat, CompactVectorStatTest, compact_vector_stat_types);
} // namespace
