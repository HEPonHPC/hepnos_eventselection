#ifndef TIMING_RECORD_FOR_BM_HPP
#define TIMING_RECORD_FOR_BM_HPP

#include <ostream>

enum class Steps {
  start,
  pre_connect_datastore,
  post_connect_datastore,
  pre_read_dataset,
  post_read_dataset,
  pre_init_asyncengine,
  post_init_asyncengine,
  pre_init_paralleleventprocessor,
  post_init_paralleleventprocessor,
  pre_check_preload,
  post_check_preload,
  pre_prepare_preload,
  post_prepare_preload,
  pre_preload,
  post_preload,
  pre_barrier,
  pre_run_benchmark,
  post_run_benchmark,
  pre_post_read_barrier,
  post_post_read_barrier,
  finish
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
    case Steps::post_connect_datastore:
      os << "post_connect_datastore";
      break;
    case Steps::pre_read_dataset:
      os << "pre_read_dataset";
      break;
    case Steps::post_read_dataset:
      os << "post_read_dataset";
      break;
    case Steps::pre_init_asyncengine:
      os << "pre_init_asyncengine";
      break;
    case Steps::post_init_asyncengine:
      os << "post_init_asyncengine";
      break;
    case Steps::pre_init_paralleleventprocessor:
      os << "pre_init_paralleleventprocessor";
      break;
    case Steps::post_init_paralleleventprocessor:
      os << "post_init_paralleleventprocessor";
      break;
    case Steps::pre_check_preload:
      os << "pre_check_preload";
      break;
    case Steps::post_check_preload:
      os << "post_check_preload";
      break;
    case Steps::pre_prepare_preload:
      os << "pre_prepare_preload";
      break;
    case Steps::post_prepare_preload:
      os << "post_prepare_preload";
      break;
    case Steps::pre_preload:
      os << "pre_preload";
      break;
    case Steps::post_preload:
      os << "post_preload";
      break;
    case Steps::pre_barrier:
      os << "pre_barrier";
      break;
    case Steps::pre_run_benchmark:
      os << "pre_run_benchmark";
      break;
    case Steps::post_run_benchmark:
      os << "post_run_benchmark";
      break;
    case Steps::pre_post_read_barrier:
      os << "pre_post_read_barrier";
      break;
    case Steps::post_post_read_barrier:
      os << "post_post_read_barrier";
      break;
    case Steps::start:
      os << "start";
      break;
  }
  return os;
}

struct timing_record_for_bm {
  double ts;
  std::size_t data;
  Steps s;
};

inline std::ostream&
operator<<(std::ostream& os, timing_record_for_bm const& r)
{
  os << r.ts << ',' << r.data << ',' << r.s;
  return os;
}

#endif
