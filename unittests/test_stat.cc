#include <unittests/test_compact_vector.hpp>

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
} // namespace
