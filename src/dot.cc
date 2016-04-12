#include <cstddef>

//------------------------------------------------------------------------------

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


