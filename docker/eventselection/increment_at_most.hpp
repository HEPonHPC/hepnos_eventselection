#ifndef INCREMENT_AT_MOST_HPP
#define INCREMENT_AT_MOST_HPP

#include <cstddef>

// increment it no more than n times, not past end and staying in the same
// subrun.
template <typename Fi>
void
increment_at_most(Fi& it, Fi end, std::size_t n)
{
  if (it == end)
    return;
  auto const sr = subrun_number(*it);
  auto const r = run_number(*it);
  for (std::size_t i = 0;
       i != n && it != end && sr == subrun_number(*it) && r == run_number(*it);
       ++i, ++it) {
  };
}

#endif
