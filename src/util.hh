#pragma once

#include <cstddef>
#include <utility>

//------------------------------------------------------------------------------

template<typename T> T square(T val) { return val * val; }

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

extern void thrash_cache(size_t);

