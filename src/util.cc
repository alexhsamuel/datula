#include <cassert>
#include <cstdlib>

#include "util.hh"

//------------------------------------------------------------------------------

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


