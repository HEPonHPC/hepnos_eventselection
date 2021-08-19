#ifndef SPLIT_CONFIGS_HPP
#define SPLIT_CONFIGS_HPP
#include <diy/mpi.hpp>

#include <assert.h>

template <typename T>
std::pair<typename T::const_iterator, typename T::const_iterator> 
split_configs(T& block_configurations, 
              int rank,
              int nranks) {
  using iter_t = typename T::const_iterator;
  // Some ranks will process M blocks; others will process M+1.
  // It is the lower-number ranks that will process more.
  std::size_t const total_num_blocks = block_configurations.size();
  auto const M =   total_num_blocks / nranks;
  auto const rem = total_num_blocks % nranks;

  // The first 'rem' ranks are doing M+1 blocks.
  if (rank < rem)
  {
    std::size_t offset = rank * (M+1);
    iter_t begin = block_configurations.begin() + offset;
    iter_t end = begin + (M+1);
    return { begin, end };
  }
  // The rest of the ranks process M blocks.
  std::size_t offset = (M+1)*rem + M * (rank - rem);
  iter_t begin = block_configurations.begin() + offset;
  iter_t end = begin + M;
  return { begin, end };
}
#endif
