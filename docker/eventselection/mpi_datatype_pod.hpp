#ifndef MPI_DATATYPE_POD_HPP
#define MPI_DATATYPE_POD_HPP

#include <diy/mpi.hpp>

namespace es {
  template <typename T>
  struct mpi_datatype_pod {
    static MPI_Datatype
    datatype()
    {
      return MPI_BYTE;
    }
    static const void*
    address(T const& x)
    {
      return &x;
    }
    static void*
    address(T& x)
    {
      return &x;
    }
    static int
    count(T const&)
    {
      return sizeof(T);
    }
  };
}
#endif
