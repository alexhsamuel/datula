#pragma once

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <utility>

#include "util.hh"

//------------------------------------------------------------------------------

using Clock = std::chrono::high_resolution_clock;
using Timestamp = std::chrono::time_point<Clock>;
using Elapsed = double;

inline Elapsed
time_since(
  Timestamp const start)
{
  return std::chrono::duration<Elapsed>(Clock::now() - start).count();
}


/*
 * Invokes `fn(args...)`, and returns the elapsed time and return value.
 */
template<typename FN, typename ...ARGS>
std::pair<Elapsed, std::result_of_t<FN&&(ARGS&&...)>>
inline time1(
  FN&& fn,
  ARGS&&... args)
{
  auto const start  = Clock::now();
  auto const result = fn(std::forward<ARGS>(args)...);
  auto const end    = Clock::now();
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
    *begin,      // FIXME: Wrong!
    *(end - 1),  // FIXME: Wrong!
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


inline
std::string
format_ns(
  Elapsed const elapsed)
{
  long const ns = (long) (elapsed * 1e9);
  std::stringstream ss;
  ss << std::setw(6) << ns << " ns";
  return ss.str();
}


template<typename T>
inline std::ostream&
operator<<(
  std::ostream& os,
  SummaryStats<T> const& stats)
{
  os << "num=" << std::setw(6) << stats.num_samples
     << " min=" << format_ns(stats.min)
     << " max=" << format_ns(stats.max)
     << " µ=" << format_ns(stats.mean)
     << " σ=" << format_ns(stats.standard_deviation);

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

class Timer
{
public:

  using SetupFn = void (*)();

  Timer(
    Elapsed const time_budget,
    double const discard_fraction=0,
    SetupFn const setup=nullptr
    )
  : time_budget_(time_budget),
    discard_fraction_(discard_fraction),
    setup_(setup)
  {
    assert(time_budget > 0);
    assert(discard_fraction >= 0);
  }

  template<typename FN, typename ...ARGS>
  SummaryStats<Elapsed>
  operator()(
    FN&& fn,
    ARGS&&... args)
  {
    std::vector<Elapsed> elapsed;

    auto const start = Clock::now();
    do {
      if (setup_ != nullptr)
        setup_();
      elapsed.push_back(time1(fn, std::forward<ARGS>(args)...).first);
    } while (time_since(start) < time_budget_);
      
    // Sort and ignore the highest and lowest.
    std::sort(elapsed.begin(), elapsed.end());
    size_t const num_discard = elapsed.size() * discard_fraction_ / 2;
    auto const begin = elapsed.begin() + num_discard;
    auto const end   = elapsed.end()   - num_discard;

    return summarize(begin, end);
  }

private:

  Elapsed const time_budget_;
  double const discard_fraction_;
  SetupFn const setup_;

};


