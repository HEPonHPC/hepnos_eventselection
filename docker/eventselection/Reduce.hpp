#ifndef DIYGEN_Reduce_HPP
#define DIYGEN_Reduce_HPP 1

#include "timing_record.hpp"
// --- callback functions ---//
//
// callback function for merge operator, called in each round of the reduction
// one block is the root of the group
// link is the neighborhood of blocks in the group
// root block of the group receives data from other blocks in the group and
// reduces the data nonroot blocks send data to the root
//

// have not yet done a local neighbor exchange yet.
// this will become a template with parameter that is the Block type

template <typename B>
void
reduceData(B* b,
           const diy::ReduceProxy& rp,
           const diy::RegularMergePartners& partners,
           std::vector<timing_record>& timingdata)
{
 std::size_t const block_id = static_cast<std::size_t>(rp.gid());
   // data is block id
  timingdata.push_back(
    {MPI_Wtime(), block_id, Step::start_reduce_data});
  // return_type is the type returned by Block::reduceData()
  using return_type = decltype(std::declval<B>().reduceData());
  using data_type = typename std::remove_const<
    typename std::remove_reference<return_type>::type>::type;

  data_type& in_vals = b->reduceData();
  // step 1: dequeue and merge
  for (std::size_t i = 0; i < rp.in_link().size(); ++i) {
    int const neighbor_gid = rp.in_link().target(i).gid;
    // we want a new event here, Step::start_dequeue_loop
    timingdata.push_back({MPI_Wtime(),
                          static_cast<std::size_t>(neighbor_gid),
                          Step::start_dequeue_loop});
    // data recorded is neighbor_gid
    if (neighbor_gid == rp.gid()) {
      auto now = MPI_Wtime();
      timingdata.push_back({now, i, Step::pre_dequeue});
      timingdata.push_back({now, 0, Step::pre_block_reduce});
      timingdata.push_back({now, block_id, Step::post_block_reduce});
      timingdata.push_back({now, rp.round(), Step::end_dequeue_loop});
      continue;
    }
    data_type tmp;
    // we can leave i here as our loop number
    timingdata.push_back({MPI_Wtime(), i, Step::pre_dequeue});
    rp.dequeue(neighbor_gid, tmp);
    // we want to record size of tmp here
    timingdata.push_back(
      {MPI_Wtime(), tmp.slices.size(), Step::pre_block_reduce});
    B::reduce(in_vals, tmp);
    auto now = MPI_Wtime();
    timingdata.push_back({now,
                          block_id, 
                          Step::post_block_reduce});
    timingdata.push_back({now, rp.round(), Step::end_dequeue_loop});
  }

  // data is Number of incoming slice IDs
  timingdata.push_back(
    {MPI_Wtime(), in_vals.slices.size(), Step::mid_reduce_data});
  // step 2: enqueue
  for (std::size_t i = 0; i < rp.out_link().size();
       ++i) // redundant since size should equal to 1
  {
    timingdata.push_back({MPI_Wtime(), i, Step::start_enqueue_loop});
    // only send to root of group, but not self
    if (rp.out_link().target(i).gid != rp.gid()) {
      timingdata.push_back(
        {MPI_Wtime(),
         static_cast<std::size_t>(rp.out_link().target(i).gid),
         Step::pre_enqueue});
      rp.enqueue(rp.out_link().target(i), in_vals);
      timingdata.push_back(
        {MPI_Wtime(), in_vals.slices.size(), Step::post_enqueue});
    }
    else {
      timingdata.push_back(
        {MPI_Wtime(),
         static_cast<std::size_t>(rp.out_link().target(i).gid),
         Step::pre_enqueue});
      timingdata.push_back(
        {MPI_Wtime(), 0, Step::post_enqueue});
    
    }
    auto now = MPI_Wtime();
    timingdata.push_back({now, block_id, Step::pre_end_enqueue_loop});
    timingdata.push_back({now, rp.round(), Step::end_enqueue_loop});
  }
  timingdata.push_back(
    {MPI_Wtime(), static_cast<std::size_t>(rp.round()), Step::end_reduce_data});
}
#endif
