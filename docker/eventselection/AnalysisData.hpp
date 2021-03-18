#ifndef ANALYSIS_DATA_HPP
#define ANALYSIS_DATA_HPP
#include <utility>
#include <vector>

#include "SliceID.hpp"
#include "mpi_datatype_pod.hpp"

#include "diy/mpi.hpp"

namespace es {

  struct AnalysisData {
    std::vector<SliceID> slices;
    void
    push_back(SliceID const& s)
    {
      slices.push_back(s);
    };
    void
    push_back(SliceID&& s)
    {
      slices.push_back(std::forward<SliceID>(s));
    };
  };

  void accumulate_into_first(AnalysisData& a, AnalysisData const& b);
}

inline void
es::accumulate_into_first(es::AnalysisData& a, es::AnalysisData const& b)
{
  a.slices.insert(a.slices.end(), b.slices.cbegin(), b.slices.cend());
}

namespace diy {

  template <>
  struct Serialization<es::AnalysisData> {

    static void
    save(diy::BinaryBuffer& bb, es::AnalysisData const& ad)
    {
      diy::save(bb, ad.slices);
    }

    static void
    load(diy::BinaryBuffer& bb, es::AnalysisData& ad)
    {
      diy::load(bb, ad.slices);
    }
  };
} // namespace diy
#endif
