#ifndef TIMING_RECORD_FOR_MPI_HPP
#define TIMING_RECORD_FOR_MPI_HPP

#include <ostream>

enum class Steps {
  start,
  pre_connect_datastore,
  pre_create_dataset,
  pre_create_asyncengine,
  pre_create_paralleleventprocessor,
  pre_preload,
  post_preload,
  begin_dowork,
  end_dowork,
  pre_process,
  post_process,
  pre_reduce_results,
  post_reduce_results,
  finish,
};

inline std::ostream&
operator<<(std::ostream& os, Steps s)
{
  switch (s) {
    case Steps::finish:
      os << "finish";
      break;
    case Steps::pre_connect_datastore:
      os << "pre_connect_datastore";
      break;
    case Steps::pre_create_dataset:
      os << "pre_create_dataset";
      break;
    case Steps::pre_create_asyncengine:
      os << "pre_create_asyncengine";
      break;
    case Steps::pre_create_paralleleventprocessor:
      os << "pre_create_paralleleventprocessor";
      break;
    case Steps::pre_preload:
      os << "pre_preload";
      break;
    case Steps::post_preload:
      os << "post_preload";
      break;
    case Steps::begin_dowork:
      os << "begin_dowork";
      break;
    case Steps::end_dowork:
      os << "end_dowork";
      break;
    case Steps::pre_process:
      os << "pre_process";
      break;
    case Steps::post_process:
      os << "post_process";
      break;
    case Steps::pre_reduce_results:
      os << "pre_reduce_results";
      break;
    case Steps::post_reduce_results:
      os << "post_reduce_results";
      break;
    case Steps::start:
      os << "start";
      break;
  }
  return os;
}

struct timing_record_for_mpi {
  double ts;
  std::size_t data;
  Steps s;
};

inline std::ostream&
operator<<(std::ostream& os, timing_record_for_mpi const& r)
{
  os << r.ts << ',' << r.data << ',' << r.s;
  return os;
}

#endif
