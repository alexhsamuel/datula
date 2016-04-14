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
  if (argc != 4) {
    std::cerr << "usage: " << argv[0] << " SIZE NUM THRASH\n";
    return EXIT_FAILURE;
  }
  auto const size   = parse_size(argv[1]);
  auto const num    = parse_size(argv[2]);
  auto const thrash = parse_size(argv[3]);

  double* const arr0 = new double[size];
  double* const arr1 = new double[size];
  for (long i = 0; i < size; ++i) {
    arr0[i] = i + 1;
    arr1[i] = 1.0 / (i + 1);
  }  

  for (long i = 0; i < num; ++i) {
    if (thrash > 0)
      thrash_cache(thrash);
    std::cout << time1(dot, size, arr0, arr1).first << std::endl;
  }
    
  return EXIT_SUCCESS;
}


