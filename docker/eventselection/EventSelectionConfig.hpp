#ifndef EVENT_SELECTION_CONFIG_HPP
#define EVENT_SELECTION_CONFIG_HPP

#include <cstdint>
#include <istream>
#include <ostream>
#include <string>

#include "mpi_datatype_pod.hpp"
#include <hepnos/SubRun.hpp>

#include <diy/mpi.hpp>

struct EventSelectionConfig {
  // The default constructed block has no work to do.
  const std::uint64_t run_num = 0;
  const std::uint64_t subrun_num = 0;
  const std::uint64_t begin_eventnum = 0;
  const std::size_t max_events = 0;
};

inline std::ostream&
operator<<(std::ostream& ost, EventSelectionConfig const& ec)
{
  ost << ec.run_num << ' ' << ec.subrun_num << ' ' << ec.begin_eventnum << ' '
      << ec.max_events;
  return ost;
}

namespace diy {
  template <>
  struct Serialization<EventSelectionConfig> {
    static void
    save(diy::BinaryBuffer& bb, const EventSelectionConfig& m)
    {
      diy::save(bb, &m, sizeof(EventSelectionConfig));
    }
    static void
    load(diy::BinaryBuffer& bb, EventSelectionConfig& m)
    {
      diy::load(bb, &m, sizeof(EventSelectionConfig));
    }
  };
  // template<>
  // struct Serialization<std::string> {
  //  static void
  //  save(diy::BinaryBuffer& bb, const std::string& s)
  //  {
  //    diy::save(bb, s.size());
  //    diy::save(bb, s.data());
  //  }
  //  static void
  //  load(diy::BinaryBuffer& bb, std::string& s)
  //  { size_t size;
  //     diy::load(bb, size);
  //     std::vector<char> data(size);
  //     diy::load(bb, &data[0], size);s.assign(&data[0], size);
  //                                                                                                                                                                                                        }
  // };

  namespace mpi {
    namespace detail {
      template <>
      struct mpi_datatype<EventSelectionConfig>
        : public es::mpi_datatype_pod<EventSelectionConfig> {};

      template <std::size_t N>
      struct mpi_datatype<std::array<char, N>> {
        static MPI_Datatype
        datatype()
        {
          return MPI_BYTE;
        }
        static const void*
        address(std::array<char, N> const& x)
        {
          return &x;
        }
        static void*
        address(std::array<char, N>& x)
        {
          return &x;
        }
        static int
        count(std::array<char, N> const&)
        {
          return sizeof(std::array<char, N>);
        }
      };
      template <>
      struct mpi_datatype<std::string> {
        static MPI_Datatype
        datatype()
        {
          return MPI_BYTE;
        }
        static const void*
        address(std::string const& x)
        {
          return &x;
        }
        static void*
        address(std::string& x)
        {
          return &x;
        }
        static int
        count(std::string const&)
        {
          return sizeof(std::string);
        }
      };
    } // namespace detail
  }   // namespace mpi
} // namepspace diy
#endif
