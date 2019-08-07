#include <unittests/test_compact_vector.hpp>

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

TEST(CompactVector, CopyMove) {
  const unsigned int       bits = 17;
  const size_t             size = 2000;
  compact::vector<int>     vector1(bits, size);
  compact::vector<int, 17> vector2(size);

  for(size_t i = 0; i < size; i++) {
    vector1[i] = i;
    vector2[i] = i;
  }

  auto cvector1(vector1);
  auto cvector2(vector2);
  EXPECT_EQ(vector1.size(), vector2.size());
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


//
// Testing compact::vector_imp::vector for different vector type, word type, bits and used bits value.
//

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
    // if(*ary != val) {
    //   std::cerr << i << ' ' << *ary << ' ' << val << std::endl;
    //   asm("int3");
    // }
      //    assert(*ary == val);
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
