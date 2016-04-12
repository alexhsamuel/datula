#include <chrono>
#include <utility>

//------------------------------------------------------------------------------

/*
 * Calls fn on args and returns the result, suppressing inlining.
 */
template<typename FN, typename ...ARGS>
__attribute((noinline)) std::result_of_t<FN&&(ARGS&&...)>
no_inline(
  FN&& fn,
  ARGS&&... args)
{
  return fn(std::forward<ARGS>(args)...);
}


//------------------------------------------------------------------------------

#include <cassert>
#include <cstdlib>
#include <iostream>

void
thrash_cache(
  size_t const size)
{
  if (size == 0)
    return;

  static unsigned char* buf = nullptr;
  static size_t buf_size = 0;

  if (buf_size < size) {
    buf = (unsigned char*) realloc(buf, size);
    assert(buf != nullptr);
    buf_size = size;
  }

  for (size_t i = 0; i < buf_size; ++i)
    buf[i] = (unsigned char) i;

  unsigned long n = 0;
  for (size_t i = 0; i < buf_size; ++i)
    n += buf[i];
  assert(n == 32640 * (size / 256) + (size % 256) * (size % 256 + 1) / 2);
}


//------------------------------------------------------------------------------

using Elapsed = double;

template<typename FN, typename ...ARGS>
std::pair<Elapsed, std::result_of_t<FN&&(ARGS&&...)>>
inline time1(
  FN volatile&& fn,
  ARGS&&... args)
{
  auto const start  = std::chrono::high_resolution_clock::now();
  auto const result = fn(std::forward<ARGS>(args)...);
  auto const end    = std::chrono::high_resolution_clock::now();
  return {std::chrono::duration<Elapsed>(end - start).count(), result};
}


//------------------------------------------------------------------------------

#include <iostream>
#include <unistd.h>

extern double dot(size_t, double const*, double const*);

int
main()
{
  size_t const num = 1024 * 1024 * 1024;
  double* const arr0 = new double[num];
  double* const arr1 = new double[num];
  for (size_t i = 0; i < num; ++i) {
    arr0[i] = i + 1;
    arr1[i] = 1.0 / (i + 1);
  }

  for (size_t i = 0; i < 16; ++i) {
    thrash_cache(1024 * 1024 * 1024);
    auto const result = time1(dot, num, arr0, arr1);
    assert(result.second == num);
    std::cout << result.first << std::endl;
  }

  return EXIT_SUCCESS;
}


