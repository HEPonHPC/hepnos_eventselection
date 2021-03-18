#ifndef SLICE_IDS_HPP
#define SLICE_IDS_HPP

#include <cstdint>
#include <diy/mpi.hpp>
#include <diy/serialization.hpp>

#include "mpi_datatype_pod.hpp"
#include "SliceID.hpp"

struct SliceIDs {
  std::vector<int> runs;
  std::vector<int> subRuns;
  std::vector<int> events;
  std::vector<int> slices;
  
  void push_back(SliceID const& a){
    runs.push_back(a.run);
    subRuns.push_back(a.subRun);
    events.push_back(a.event);
    slices.push_back(a.slice);
  }
  
  void reserve(std::size_t n) {
    runs.reserve(n);
    subRuns.reserve(n);
    events.reserve(n);
    slices.reserve(n);
  }

  std::size_t size() const {
    return runs.size();
  }

  friend SliceIDs operator+(SliceIDs const& a, SliceIDs const& b) {
    SliceIDs results{a};
    results.runs.insert(results.runs.end(), b.runs.begin(), b.runs.end());
    results.subRuns.insert(results.subRuns.end(), b.subRuns.begin(), b.subRuns.end());
    results.events.insert(results.events.end(), b.events.begin(), b.events.end());
    results.slices.insert(results.slices.end(), b.slices.begin(), b.slices.end());
    return results;
  }
};

inline std::ostream&
operator<<(std::ostream& os, SliceIDs const& s)
{
  for (auto i=0; i<s.size(); ++i) {
    os << s.runs[i] << ' ' << s.subRuns[i] << ' ' << s.events[i] << ' ' << s.slices[i] << '\n';
  }
  return os;
}

#endif
