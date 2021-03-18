#ifndef TIMING_RECORD_HPP
#define TIMING_RECORD_HPP

#include <ostream>

enum class Step {
  end_dequeue_loop,
  end_enqueue_loop,
  end_reduce_data,
  finish,
  mid_reduce_data,
  post_block_configurations,
  post_block_reduce,
  post_broadcast,
  post_create_records,
  post_dataset,
  post_decompose,
  post_enqueue,
  post_execute_block,
  post_fill_records,
  post_process_block,
  post_process_slices,
  post_reduction,
  pre_block_reduce,
  pre_create_partners,
  pre_create_records,
  pre_decompose,
  pre_dequeue,
  pre_end_enqueue_loop,
  pre_enqueue,
  pre_execute_block,
  pre_process_block,
  pre_reduction,
  start,
  start_enqueue_loop,
  start_dequeue_loop,
  start_reduce_data
};

inline std::ostream&
operator<<(std::ostream& os, Step s)
{
  switch (s) {
    case Step::end_dequeue_loop:
      os << "end_dequeue_loop";
      break;
    case Step::end_enqueue_loop:
      os << "end_enqueue_loop";
      break;
    case Step::end_reduce_data:
      os << "end_reduce_data";
      break;
    case Step::finish:
      os << "finish";
      break;
    case Step::mid_reduce_data:
      os << "mid_reduce_data";
      break;
    case Step::post_block_configurations:
      os << "post_block_configurations";
      break;
    case Step::post_block_reduce:
      os << "post_block_reduce";
      break;
    case Step::post_broadcast:
      os << "post_broadcast";
      break;
    case Step::post_create_records:
      os << "post_create_records";
      break;
    case Step::post_dataset:
      os << "post_dataset";
      break;
    case Step::post_decompose:
      os << "post_decompose";
      break;
    case Step::post_enqueue:
      os << "post_enqueue";
      break;
    case Step::post_execute_block:
      os << "post_execute_block";
      break;
    case Step::post_fill_records:
      os << "post_fill_records";
      break;
    case Step::post_process_block:
      os << "post_process_block";
      break;
    case Step::post_process_slices:
      os << "post_process_slices";
      break;
    case Step::post_reduction:
      os << "post_reduction";
      break;
    case Step::pre_block_reduce:
      os << "pre_block_reduce";
      break;
    case Step::pre_create_partners:
      os << "pre_create_partners";
      break;
    case Step::pre_create_records:
      os << "pre_create_records";
      break;
    case Step::pre_decompose:
      os << "pre_decompose";
      break;
    case Step::pre_end_enqueue_loop:
      os << "pre_end_enqueue_loop";
      break;
    case Step::pre_enqueue:
      os << "pre_enqueue";
      break;
    case Step::pre_dequeue:
      os << "pre_dequeue";
      break;
    case Step::pre_execute_block:
      os << "pre_execute_block";
      break;
    case Step::pre_process_block:
      os << "pre_process_block";
      break;
    case Step::pre_reduction:
      os << "pre_reduction";
      break;
    case Step::start:
      os << "start";
      break;
    case Step::start_enqueue_loop:
      os << "start_enqueue_loop";
      break;
    case Step::start_dequeue_loop:
      os << "start_dequeue_loop";
      break;
    case Step::start_reduce_data:
      os << "start_reduce_data";
      break;
  }
  return os;
}

struct timing_record {
  double ts;
  std::size_t data;
  Step s;
};

inline std::ostream&
operator<<(std::ostream& os, timing_record const& r)
{
  os << r.ts << ',' << r.data << ',' << r.s;
  return os;
}

#endif
