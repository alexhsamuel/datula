#include <cstddef>

#include "timing.hh"

//------------------------------------------------------------------------------

__attribute((noinline)) 
double
dot(
  size_t const num,
  double const* const arg0,
  double const* const arg1)
{
  double result = 0;
  for (size_t i = 0; i < num; ++i)
    result += arg0[i] * arg1[i];
  return result;
}


int
main(
  int const argc,
  char const* const* const argv)
{
  size_t const num = 32 * 1024 * 1024;
  double* const arr0 = new double[num];
  double* const arr1 = new double[num];
  for (size_t i = 0; i < num; ++i) {
    arr0[i] = i + 1;
    arr1[i] = 1.0 / (i + 1);
  }

  timing_main(argc, argv, dot, num, arr0, arr1);
}
