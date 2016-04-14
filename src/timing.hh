#pragma once

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <utility>

#include "util.hh"

//------------------------------------------------------------------------------

using Elapsed = double;

template<typename FN, typename ...ARGS>
std::pair<Elapsed, std::result_of_t<FN&&(ARGS&&...)>>
inline time1(
  FN&& fn,
  ARGS&&... args)
{
  auto const start  = std::chrono::high_resolution_clock::now();
  auto const result = fn(std::forward<ARGS>(args)...);
  auto const end    = std::chrono::high_resolution_clock::now();
  return {std::chrono::duration<Elapsed>(end - start).count(), result};
}


//------------------------------------------------------------------------------

struct Timing {
  size_t num_samples;
  Elapsed min;
  Elapsed max;
  Elapsed mean;
  Elapsed standard_deviation;
};


inline Timing
operator/(
  Timing const& timing,
  size_t const scale)
{
  return {
    timing.num_samples,
    timing.min / scale,
    timing.max / scale,
    timing.mean / scale,
    timing.standard_deviation / scale
  };
}


inline std::ostream&
operator<<(
  std::ostream& os,
  Timing const& timing)
{
  os << "num_samples=" << timing.num_samples
     << " min=" << timing.min
     << " max=" << timing.max
     << " mean=" << timing.mean
     << " standard_deviation=" << timing.standard_deviation;

  return os;
}


//------------------------------------------------------------------------------

class Timeit
{
public:

  /*
   * @num_samples
   *   The number of timing samples to use.
   * @num_discard
   *   The number of (each of) highest and lowest samples to discard.
   */
  Timeit(
    size_t const num_samples,
    size_t const num_discard=0)
  : num_samples_(num_samples),
    num_discard_(num_discard)
  {
    assert(num_samples_ > num_discard_ * 2);
  }

  template<typename FN, typename ...ARGS>
  inline Timing
  operator()(
    FN&& fn,
    ARGS&&... args)
  {
    std::vector<Elapsed> elapsed;
    while (elapsed.size() < num_samples_) {
      thrash_cache(64 * 1024 * 1024);  // FIXME
      elapsed.push_back(time1(fn, std::forward<ARGS>(args)...).first);
    }
      
    return calculate(std::move(elapsed));
  }

private:

  inline Timing
  calculate(
    std::vector<Elapsed>&& elapsed)
  {
    // Sort and ignore the highest and lowest.
    std::sort(elapsed.begin(), elapsed.end());
    auto const begin = elapsed.begin() + num_discard_;
    auto const end   = elapsed.end()   - num_discard_;

    // Compute moments.
    size_t  m0 = 0;
    Elapsed m1 = 0;
    Elapsed m2 = 0;
    for (auto i = begin; i < end; ++i) {
      m0 += 1;
      m1 += *i;
      m2 += *i * *i;
    }

    return {
      m0,
      *begin,
      *(end - 1),
      m1 / m0,
      sqrt(m2 / m0 - square(m1 / m0)) * m0 / (m0 - 1)
    };
  }

  size_t const num_samples_;
  size_t const num_discard_;

};


//------------------------------------------------------------------------------

template<typename FN, typename ...ARGS>
int
timing_main(
  int const argc,
  char const* const* const argv,
  FN&& fn,
  ARGS&&... args)
{
  Timeit timeit{16, 2};

  for (size_t i = 0; i < 30; ++i) {
    size_t const n = 1 << i;
    auto const timing = timeit(fn, std::forward<ARGS>(args)...);
    std::cout << n << ": " << timing / n << std::endl;
  }

  return EXIT_SUCCESS;
}


