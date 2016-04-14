#include <cstddef>
#include <iomanip>
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

  {
    std::cout << "without cache flush:" << std::endl;
    Timer timer{1.0, 0.1, nullptr};
    for (size_t s = 0; s < 26; ++s) {
      auto const n = 1 << s;
      auto const stats = timer(dot, n, arr0, arr1);
      std::cout << std::setw(10) << n << ": " 
                << stats << " || " << stats / n
                << std::endl;
    }
  }
    
  {
    std::cout << "with cache flush:" << std::endl;
    Timer timer{1.0, 0.1, []() { thrash_cache(6 * 1024 * 1024); }};
    for (size_t s = 0; s < 26; ++s) {
      auto const n = 1 << s;
      auto const stats = timer(dot, n, arr0, arr1);
      std::cout << std::setw(10) << n << ": " 
                << stats << " || " << stats / n
                << std::endl;
    }
  }
    
  return EXIT_SUCCESS;
}


