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

/*
 * Invokes `fn(args...)`, and returns the elapsed time and return value.
 */
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

template<typename T>
struct SummaryStats
{
  size_t num_samples;
  T min;
  T max;
  T mean;
  T standard_deviation;
};


/*
 * Computes summary statistics over a number of sample values.
 */
template<typename ITER>
auto
summarize(
  ITER begin,
  ITER end)
{
  using Value = typename std::iterator_traits<ITER>::value_type;

  // Compute moments.
  size_t m0 = 0;
  Value m1 = 0;
  Value m2 = 0;
  for (auto i = begin; i < end; ++i) {
    m0 += 1;
    m1 += *i;
    m2 += *i * *i;
  }

  return SummaryStats<Value>{
    m0,
    *begin,
    *(end - 1),
    m1 / m0,
    sqrt(m2 / m0 - square(m1 / m0)) * m0 / (m0 - 1)
  };
}


template<typename T>
inline SummaryStats<T>
operator/(
  SummaryStats<T> const& stats,
  size_t const scale)
{
  return {
    stats.num_samples,
    stats.min / scale,
    stats.max / scale,
    stats.mean / scale,
    stats.standard_deviation / scale
  };
}


template<typename T>
inline std::ostream&
operator<<(
  std::ostream& os,
  SummaryStats<T> const& stats)
{
  os << "num=" << stats.num_samples
     << " min=" << stats.min
     << " max=" << stats.max
     << " µ=" << stats.mean
     << " σ=" << stats.standard_deviation;

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
  inline SummaryStats<Elapsed>
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

  inline SummaryStats<Elapsed>
  calculate(
    std::vector<Elapsed>&& elapsed)
  {
    // Sort and ignore the highest and lowest.
    std::sort(elapsed.begin(), elapsed.end());
    auto const begin = elapsed.begin() + num_discard_;
    auto const end   = elapsed.end()   - num_discard_;
    return summarize(begin, end);
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


