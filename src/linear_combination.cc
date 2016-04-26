#include <cstddef>
#include <iomanip>
#include <unistd.h>

#include "timing.hh"

//------------------------------------------------------------------------------

__attribute((noinline)) 
double
linear_combination(
  size_t const num,
  double const* const coefficients,
  size_t const len,
  double const* const* const samples,
  double* const result)
{
  double res;
  for (size_t i = 0; i < len; ++i) {
    res = 0;
    for (size_t c = 0; c < num; ++c)
      res += coefficients[c] * samples[c][i];
    result[i] = res;
  }
  return res;
}


int
main(
  int const argc,
  char const* const* const argv)
{
  if (argc != 2) {
    std::cerr << "usage: " << argv[0] << " NUM-COLUMNS\n";
    return 1;
  }
  long const num_columns = atol(argv[1]);
  if (num_columns < 1) {
    std::cerr << "NUM-COLUMNS must be positive\n";
    return 1;
  }
  size_t const num = num_columns;

  size_t const max_len = 1 << 24;

  double* coefficients = new double[num];
  for (size_t c = 0; c < num; ++c) 
    coefficients[c] = 1.0 / (c + 1);

  double** samples = new double*[num];
  for (size_t c = 0; c < num; ++c) {
    double* sample = new double[max_len];
    for (size_t i = 0; i < max_len; ++i)
      sample[i] = i + 1;
    samples[c] = sample;
  }

  double* result = new double[max_len];

  Timer timer{1.0, 0.1, nullptr};
  for (size_t len = 1024; len <= max_len; len <<= 1) {
    auto const stats 
      = timer(linear_combination, num, coefficients, len, samples, result);
    std::cout << std::setw(10) << len << ": " 
              << stats << " || " << stats / len
              << std::endl;
  }
    
  return EXIT_SUCCESS;
}


