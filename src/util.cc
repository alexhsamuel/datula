#include <cassert>
#include <cctype>
#include <cstdlib>
#include <string>

#include "util.hh"

//------------------------------------------------------------------------------

long
parse_size(
  std::string const& str)
{
  // FIXME: Throw instead of assert.
  assert(str.size() > 0);

  auto length = str.size();
  long scale = 1;
  auto last = str[length - 1];
  if (isdigit(last)) 
    ;
  else {
    switch (tolower(last)) {
    case 'k': scale = 1024; break;
    case 'm': scale = 1024 * 1024; break;
    case 'g': scale = 1024 * 1024 * 1024; break;
    default:
      assert(false);
    }
    length--;
  }

  char const* const start = str.c_str();
  char const* end;
  long value = scale * strtol(start, const_cast<char**>(&end), 10);
  assert(end == start + length);
  return value;
}


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


