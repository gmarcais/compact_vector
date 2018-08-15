#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>

#include <compact_vector.hpp>
#include <misc.hpp>

using namespace std::chrono;

struct timer {
  high_resolution_clock::time_point start;
  timer()
    : start(high_resolution_clock::now())
  { }
};
std::ostream& operator<<(std::ostream& os, timer& t) {
  auto end_time = high_resolution_clock::now();
  os << duration_cast<duration<double>>(end_time - t.start).count();
  t.start = end_time;
  return os;
}

template<typename T>
void flip_bits(const std::vector<size_t>& order, T& v) {
  timer t;
  for(auto i : order)
    v[i] = 1;
  std::cout << ' ' << t;
}

int main(int argc, char *argv[]) {
  auto                    prg  = seeded_prg<std::mt19937_64>();
  static constexpr size_t size = 100000000;

  timer shuffle_time;
  std::vector<size_t>     order(size);
  for(size_t i = 0; i < size; ++i)
    order[i] = i;
  std::shuffle(order.begin(), order.end(), prg);
  std::cout << "Shuffle: " << shuffle_time << std::endl;

  { std::cout << "std::vector<bool>:";
    timer t;
    { std::vector<bool> v(size);
      std::cout << ' ' << t;
      flip_bits(order, v);
    }
    std::cout << ' ' << t << std::endl;
  }

  { std::cout << "compact::vector<unsigned char, 1>:";
    timer t;
    { compact::vector<unsigned char, 1> v(size);
      std::cout << ' ' << t;
      flip_bits(order, v);
    }
    std::cout << ' ' << t << std::endl;
  }

  { std::cout << "compact::vector<unsigned char>(1):";
    timer t;
    { compact::vector<unsigned char> v(1, size);
      std::cout << ' ' << t;
      flip_bits(order, v);
    }
    std::cout << ' ' << t << std::endl;
  }

  return 0;
}
