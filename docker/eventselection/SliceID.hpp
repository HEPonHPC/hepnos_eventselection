#ifndef SLICE_ID_HPP
#define SLICE_ID_HPP

#include <cstdint>
#include <diy/mpi.hpp>
#include <diy/serialization.hpp>

#include "mpi_datatype_pod.hpp"

struct SliceID {
  std::uint64_t run;
  std::uint64_t subRun;
  std::uint64_t event;
  std::uint64_t slice;
};

inline bool
operator==(SliceID const& a, SliceID const& b)
{
  return a.run == b.run && a.subRun == b.subRun && a.event == b.event &&
         a.slice == b.slice;
}

inline bool
operator<(SliceID const& a, SliceID const& b)
{
  if (a.run != b.run)
    return a.run < b.run;
  if (a.subRun != b.subRun)
    return a.subRun < b.subRun;
  if (a.event != b.event)
    return a.event < b.event;
  return a.slice < b.slice;
}

namespace diy {
  template <>
  struct Serialization<SliceID> {
    static void
    save(diy::BinaryBuffer& bb, const SliceID& s)
    {
      diy::save(bb, s.run);
      diy::save(bb, s.subRun);
      diy::save(bb, s.event);
      diy::save(bb, s.slice);
    }
    static void
    load(diy::BinaryBuffer& bb, SliceID& s)
    {
      diy::load(bb, s.run);
      diy::load(bb, s.subRun);
      diy::load(bb, s.event);
      diy::load(bb, s.slice);
    }
  };

  namespace mpi {
    namespace detail {
      template <>
      struct mpi_datatype<SliceID> : public es::mpi_datatype_pod<SliceID> {};
    }
  }
}
#endif
