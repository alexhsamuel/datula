#include <cstddef>
#include <unistd.h>

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
  size_t const num = 1 << 25;
  double* const arr0 = new double[num];
  double* const arr1 = new double[num];
  for (size_t i = 0; i < num; ++i) {
    arr0[i] = i + 1;
    arr1[i] = 1.0 / (i + 1);
  }

  Timer timer{1.0, 0.1, nullptr};
  for (size_t s = 0; s < 26; ++s) 
    std::cout << s << ": " << timer(dot, 1 << s, arr0, arr1) << std::endl;
    
  return EXIT_SUCCESS;
}


