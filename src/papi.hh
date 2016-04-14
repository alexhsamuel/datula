#include <cassert>
#include <papi.h>
#include <vector>

//------------------------------------------------------------------------------

inline void
initialize()
{
  static bool initialized = false;
  if (!initialized) {
    auto ret = PAPI_library_init(PAPI_VER_CURRENT);
    assert(ret == PAPI_VER_CURRENT);
    initialized = true;
  }
}


using Event = int;
using Counter = long long;

class PapiTimer
{
public:

  PapiTimer() {}
  PapiTimer& add(int const event) { events_.push_back(event); return *this; }

  template<typename FN, typename ...ARGS>
  __attribute((noinline))
  std::pair<std::vector<Counter>, std::result_of_t<FN&&(ARGS&&...)>>
  operator()(
    FN&& fn,
    ARGS&&... args)
  {
    auto const num_events = events_.size();
    std::vector<Counter> counters(num_events);

    initialize();
    auto ret = PAPI_start_counters(&*events_.begin(), events_.size());
    assert(ret == PAPI_OK);
    auto const result = fn(std::forward<ARGS>(args)...);
    ret = PAPI_stop_counters(&*counters.begin(), num_events);
    assert(ret == PAPI_OK);

    return {std::move(counters), result};
  }

private:

  std::vector<Event> events_;

};


