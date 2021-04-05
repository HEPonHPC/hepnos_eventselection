#ifndef CONFIG_BLOCK_ADDER_HPP
#define CONFIG_BLOCK_ADDER_HPP

// An object of type ConfigBlockAdder is used to create all the blocks that
// the current MPI rank is to process.
//
// diy::decompose needs to have a function defined to create a block.
// Here, it is wrapped in an object to add blocks with an overloaded () operator
//
// Note that an instance of ConfigBlockAdder contains a copy of the set of
// configuration objects it is to use. If the configuration objects are large,
// this might not be optimal. Consider having ConfigBlockAdder just hold
// the begin and end iterators for the range of configurations it will use.

// TODO: Put the right headers here!
//
#include <ostream>
#include "diy/fmt/format.h"

template <typename Bounds, typename Link, typename B, typename Cs>
struct ConfigBlockAdder {
  using link_type = Link;
  using bounds_type = Bounds;
  using configs_type = Cs;
  using block_type = B; 
  using Iter = typename configs_type::const_iterator;

  diy::Master& master;
  configs_type configs;
  Iter current;
  diy::mpi::communicator* world;
  std::ostream* pdebug;

  ConfigBlockAdder(diy::Master& master_,
                   Iter st,
                   Iter en,
                   diy::mpi::communicator* pw,
                   std::ostream* pdebug) :
    master(master_),
    configs(st, en),
    current(st),
    world(pw),
    pdebug(pdebug)
  {
    if (pdebug) { 
      *pdebug << fmt::format(
      "rank: {} should create {} non-default blocks\n", 
      world->rank(), configs.size());
      pdebug->flush();
    }
  }

  // this is the function that is needed for diy::decompose
  // This is the function that creates the block objects.
  void operator()(
    int gid,                         // block global id
    const bounds_type& core,         // block bounds without any ghost added
    const bounds_type& /* bounds */, // block bounds including any
                                     // ghost region added
    const bounds_type& /* domain */, // global data bounds
    const link_type& link)           // neighborhood
  {
    // If pos is already at the end of the sequence of configs,
    // we make no block.
    if (current == configs.end()) return;

    if (pdebug) { 
      *pdebug << "Creating new block for config: '" << *current << "'" << std::endl;
    }
    // Consider using unique_ptr to hold the block, so that an
    // exception throw before 'diy::Master::add' is called will not
    // leak any memory.
    block_type* b = new block_type(core, *current, gid);
    link_type* l = new link_type(link);

    // Add the block and link we have just created to master, and prepare for
    // the next block.
    master.add(gid, b, l);
    ++current;
  }
};

#endif
