#include <unittests/test_compact_vector.hpp>

#include "allocator_fill_random.hpp"

namespace test_compact_vector {
std::mt19937_64 prg = seeded_prg<std::mt19937_64>();
};

namespace {
using test_compact_vector::prg;

TEST(CompactVector, RequiredBits) {
  for(unsigned int i = 1; i < sizeof(size_t) * 8 - 1; ++i) {
    size_t s = (size_t)1 << i;
    EXPECT_EQ(i,     compact::vector<uint64_t>::required_bits(s));
    EXPECT_EQ(i + 1, compact::vector<int64_t>::required_bits(s));
    EXPECT_EQ(i + 1, compact::vector<uint64_t>::required_bits(s + 1));
    EXPECT_EQ(i + 2, compact::vector<int64_t>::required_bits(s + 1));
  }
} // CompactVector.RequiredBits

class CompactVectorFixture : public ::testing::Test {
protected:
  static constexpr unsigned int     bits = 17;
  static constexpr size_t           size = 2000;

  typedef compact::vector<int, 0, uint64_t, allocator_fill_random<uint64_t>> vector_dyn;
  typedef compact::vector<int, bits, uint64_t, allocator_fill_random<uint64_t>> vector_stat;

  allocator_fill_random<uint64_t> allocator_fr;

  vector_dyn         vector1;
  vector_stat        vector2;
  const vector_dyn&  vector1c;
  const vector_stat& vector2c;

  CompactVectorFixture()
    : vector1(bits, size, allocator_fr)
    , vector2(size, allocator_fr)
    , vector1c(vector1)
    , vector2c(vector2)
  { }

  void SetUp() override {
    for(size_t i = 0; i < size; i++) {
      vector1[i] = i;
      vector2[i] = i;
    }
  }
};
const unsigned int CompactVectorFixture::bits;
const size_t       CompactVectorFixture::size;

TEST_F(CompactVectorFixture, CopyMove) {
  auto cvector1(vector1);
  auto cvector2(vector2);
  EXPECT_EQ(size, cvector1.size());
  EXPECT_EQ(size, cvector2.size());
  for(size_t i = 0; i < size; i++) {
    EXPECT_EQ(vector1[i], cvector1[i]);
    EXPECT_EQ(vector2[i], cvector2[i]);
  }

  auto mvector1(std::move(vector1));
  auto mvector2(std::move(vector2));
  EXPECT_EQ(size, mvector1.size());
  EXPECT_EQ(size, mvector2.size());
  for(size_t i = 0; i < size; i++) {
    EXPECT_EQ((int)i, mvector1[i]);
    EXPECT_EQ((int)i, mvector2[i]);
  }
} // CompactVector.CopyMove

TEST_F(CompactVectorFixture, CopyMoveOp) {
  decltype(vector1) cvector1(bits);
  decltype(vector2) cvector2;

  EXPECT_EQ(0, cvector1.size());
  EXPECT_EQ(0, cvector2.size());

  cvector1 = vector1;
  cvector2 = vector2;

  EXPECT_EQ(size, cvector1.size());
  EXPECT_EQ(size, cvector2.size());
  for(size_t i = 0; i < size; i++) {
    EXPECT_EQ(vector1[i], cvector1[i]);
    EXPECT_EQ(vector2[i], cvector2[i]);
  }

  decltype(vector1) mvector1(bits);
  decltype(vector2) mvector2;

  mvector1 = std::move(cvector1);
  mvector2 = std::move(cvector2);

  EXPECT_EQ(size, mvector1.size());
  EXPECT_EQ(size, mvector2.size());
  for(size_t i = 0; i < size; i++) {
    EXPECT_EQ(vector1[i], mvector1[i]);
    EXPECT_EQ(vector2[i], mvector2[i]);
  }
} // CompactVector.CopyMoveOp

TEST_F(CompactVectorFixture, At) {
  EXPECT_NO_THROW(
                  for(size_t i = 0; i < size; ++i) {
                    EXPECT_EQ((int)i, vector1.at(i));
                    EXPECT_EQ((int)i, vector1c.at(i));
                    EXPECT_EQ((int)i, vector2.at(i));
                    EXPECT_EQ((int)i, vector2c.at(i));
                  }
                  );

  for(size_t i = size; i < 2*size; ++i) {
    EXPECT_THROW(vector1.at(i), std::out_of_range);
    EXPECT_THROW(vector1c.at(i), std::out_of_range);
    EXPECT_THROW(vector2.at(i), std::out_of_range);
    EXPECT_THROW(vector2c.at(i), std::out_of_range);
  }
} // CompactVector.At

TEST_F(CompactVectorFixture, Assign) {
  std::vector<int> nv1(2*size);
  std::vector<int> nv2(size /2);
  const int mask = (1 << (bits - 1)) - 1;

  for(size_t i = 0; i < nv1.size(); ++i)
    nv1[i] = (3 * i + 1) & mask;
  for(size_t i = 0; i < nv2.size(); ++i)
    nv2[i] = (5 * i - 2) & mask;

  vector1.assign(nv1.cbegin(), nv1.cend());
  EXPECT_EQ(nv1.size(), vector1.size());
  EXPECT_TRUE(std::equal(nv1.cbegin(), nv1.cend(), vector1.begin()));
  EXPECT_TRUE(std::equal(nv1.cbegin(), nv1.cend(), vector1.cbegin()));

  vector2.assign(nv2.cbegin(), nv2.cend());
  EXPECT_EQ(nv2.size(), vector2.size());
  EXPECT_TRUE(std::equal(nv2.cbegin(), nv2.cend(), vector2.cbegin()));

  vector1.assign(3 * size, -2);
  EXPECT_EQ(3 * size, vector1.size());
  for(size_t i = 0; i < vector1.size(); ++i)
    EXPECT_EQ(vector1[i], -2);

  auto il = { -5, 2, 10, -7 };
  vector1.assign(il);
  EXPECT_EQ(vector1.size(), il.size());
  EXPECT_TRUE(std::equal(il.begin(), il.end(), vector1.begin()));
  EXPECT_TRUE(std::equal(il.begin(), il.end(), vector1.cbegin()));
} // CompactVectorFixture.Assign

TEST_F(CompactVectorFixture, Resize) {
  vector1.resize(size / 2);
  EXPECT_EQ(size / 2, vector1.size());
  for(size_t i = 0; i < vector1.size(); ++i)
    EXPECT_EQ((int)i, vector1[i]);

  vector1.resize(size);
  EXPECT_EQ(size, vector1.size());
  for(size_t i = 0; i < size / 2; ++i)
    EXPECT_EQ((int)i, vector1[i]);
  for(size_t i = size / 2; i < size; ++i)
    EXPECT_EQ(0, vector1[i]);

  vector1.resize(2*size, -1);
  EXPECT_EQ(2*size, vector1.size());
  for(size_t i = 0; i < size / 2; ++i)
    EXPECT_EQ((int)i, vector1[i]);
  for(size_t i = size / 2; i < size; ++i)
    EXPECT_EQ(0, vector1[i]);
  for(size_t i = size; i < 2*size; ++i)
    EXPECT_EQ(-1, vector1[i]);
} // CompactVectorFixture.Resize

TEST_F(CompactVectorFixture, Emplace) {
  static const size_t nb = 10;
  for(size_t i = 0; i < nb; ++i) {
    const auto it = vector1.emplace(vector1.begin() + 2*i, -(int)i);
    EXPECT_EQ(2*(ssize_t)i, it - vector1.begin());
    EXPECT_EQ(-(int)i, *it);
    EXPECT_EQ(size + i + 1, vector1.size());
    for(size_t j = 0; j <= i; ++j)
      EXPECT_EQ(-(int)j, vector1[2*j]);
  }


  for(size_t i = 0; i < nb; ++i) {
    const auto it = vector1.emplace(vector1.end() - 2 * i, -(int)i);
    EXPECT_EQ(2*(ssize_t)i + 1, vector1.end() - it);
    EXPECT_EQ(-(int)i, *it);
    EXPECT_EQ(size + i + 1 + nb, vector1.size());
    for(size_t j = 0; j <= i; ++j)
      EXPECT_EQ(-(int)j, vector1[vector1.size() - 1 - 2*j]);
  }
} // CompactVectorFixture.Emplace

TEST_F(CompactVectorFixture, Erase) {
  { const auto it = vector1.erase(vector1.begin());
    EXPECT_EQ(it, vector1.begin());
    for(size_t i = 0; i < vector1.size(); ++i)
      EXPECT_EQ((int)i + 1, vector1[i]);
  }

  static const size_t start = 10, end = 20;
  { const auto it = vector1.erase(vector1.cbegin() + start, vector1.cbegin() + end);
    EXPECT_EQ(it, vector1.begin() + start);
    for(size_t i = 0; i < (size_t)start; ++i)
      EXPECT_EQ((int)i + 1, vector1[i]);
    for(size_t i = start; i < vector1.size(); ++i)
      EXPECT_EQ((int)i + 1 + end - start, vector1[i]);
  }
} // CompactVectorFixture.Erase


//
// Testing compact::vector_imp::vector for different vector type, word type, bits and used bits value.
//

TEST(CompactIterator, Nullptr) {
  compact::iterator<int> it(nullptr);

  EXPECT_EQ(it, nullptr);
  EXPECT_EQ(nullptr, it);
} // CompactVector.Pointer

void set_values(int thid, int nb_threads, compact::vector<int>::iterator ary, size_t size) {
  typedef compact::parallel_iterator_traits<compact::vector<int>::iterator>::type pary_type;
  pary_type pary(ary);

  for(int i = 0; i < 1000; ++i)
    for(size_t j = thid; j < size; j += nb_threads)
      pary[j] = i + 1;
}

TEST(CompactVector, MultiThread) {
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
    if(i % 128 == 0)
      std::this_thread::sleep_for(std::chrono::microseconds(dist(gen)));
  }
}

TEST(CompactVector, CAS) {
  const size_t size       = 1024 * 1024;
  const int    nb_threads = 4;
  const int    bits       = 3;

  std::vector<unsigned int> ptr(size, 0);
  typedef compact::cas_vector<unsigned int> compact_vector_type;
  typedef compact_vector_type::iterator compact_iterator_type;
  compact_vector_type vector1(bits, size);
  std::fill(vector1.begin(), vector1.end(), 0);

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
  static_assert(std::is_same<decltype(vector1.cbegin()), decltype(vector1.cend())>::value, "Type begin and end");
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

// TEST_F(CompactVectorFixture, ReverseIterators) {
//   auto r1 = vector1.rbegin();
//   auto e1 = vector1.rend();
//   // auto rc1 = vector1.crbegin();
//   // auto re1 = vector1.crend();
//   // auto cr1 = vector1c.rbegin();
//   // auto ce1 = vector1c.rend();

//   for(size_t i = size; i > 0; --i, --r1) { // , --rc1, --cr1) {
//     EXPECT_FALSE(r1 == e1);
//     // EXPECT_FALSE(rc1 == re1);
//     // EXPECT_FALSE(cr1 == ce1);
//     EXPECT_EQ(vector1[i-1], *r1);
//     // EXPECT_EQ(vector1[i-1], *rc1);
//     // EXPECT_EQ(vector1[i-1], *cr1);
//   }

//   EXPECT_TRUE(r1 == e1);
//   // EXPECT_TRUE(rc1 == re1);
//   // EXPECT_TRUE(cr1 == e1);
// }

} // empty namespace
