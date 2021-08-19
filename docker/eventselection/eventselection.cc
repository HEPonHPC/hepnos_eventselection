#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <numeric>
#include <ostream>
#include <random>
#include <stdexcept>
#include <typeinfo>
#include <vector>

#include <diy/assigner.hpp>
#include <diy/decomposition.hpp>
#include <diy/master.hpp>
#include <diy/mpi.hpp>
#include <diy/partners/merge.hpp>
#include <diy/reduce.hpp>
#include <diy/serialization.hpp>

#include <hepnos.hpp>
#include <hepnos/DataSet.hpp>
#include <hepnos/DataStore.hpp>
#include <hepnos/Event.hpp>
#include <hepnos/EventSet.hpp>
#include <hepnos/ItemType.hpp>
#include <hepnos/Run.hpp>
#include <hepnos/RunSet.hpp>
#include <hepnos/SubRun.hpp>

#include "ConfigBlockAdder.hpp"
#include "GenericBlock.hpp"
#include "split_configs.hpp"
#include "AnalysisData.hpp"
#include "EventSelectionConfig.hpp"
#include "Reduce.hpp"
#include "SliceID.hpp"
#include "dataproducts.hpp"
#include "increment_at_most.hpp"
#include "timing_record.hpp"

#include "create_records_dev.hpp"
#include "dataformat/rec_hdr.hpp"
#include "mpi_datatype_pod.hpp"

#include "disable_depman.hpp"

#include "CLI11.hpp"

using namespace std;

/*
  Templates that handle standard boilerplate code are needed for:
  1) blocks
  2) management of master, assigner, and decomposer
  3) reduction operation
  4) perhaps defining custom data types to be sent around
  5) helpers for the "foreach" processing
 */

typedef diy::DiscreteBounds Bounds;
typedef diy::RegularGridLink Link;

namespace hepnos {
  auto
  event_number(Event const& e)
  {
    return e.number();
  }
  auto
  subrun_number(Event const& e)
  {
    return e.subrun().number();
  }
  auto
  run_number(Event const& e)
  {
    return e.subrun().run().number();
  }
}
using EventSelectionConfigs = std::vector<EventSelectionConfig>;

// Create a configuration object for each block that will do work.
EventSelectionConfigs
calculate_block_configs(hepnos::DataSet const& ds,
                        std::size_t max_events,
                        const int target_id,
                        std::ostream* pdebug)
{
  EventSelectionConfigs es_cfgs;
  if (max_events == 0)
    return es_cfgs;

  // use target ID to get all the events stored in that target.
  // if there are fewer subruns, then every target may not have
  // any subruns
  auto const event_set = ds.events(target_id);
  auto ievt = event_set.begin();
  auto const iend = event_set.end();
  while (ievt != iend) {
    std::uint64_t begin_eventnum = ievt->number();
    if (pdebug) {
      *pdebug << "calculate_block_config " << target_id << ", "
              << begin_eventnum << '\n';
      *pdebug << "Event ID " << ievt->subrun().run().number() << " "
              << ievt->subrun().number() << " " << ievt->number() << '\n';
    }
    es_cfgs.push_back(EventSelectionConfig{ievt->subrun().run().number(),
                                           ievt->subrun().number(),
                                           begin_eventnum,
                                           max_events});
    // Increment ievt at most max_events times, but not past iend
    // we may want to modify this to increment not past subrun boundry
    increment_at_most(ievt, iend, max_events);
  }
  return es_cfgs;
};

// ----This is common to all introduced types - can it be automated? ------

using Block = GenericBlock<Bounds, EventSelectionConfig, es::AnalysisData>;
using AddBlock = ConfigBlockAdder<Bounds, Link, Block, EventSelectionConfigs>;

void
print_event(hepnos::Event const& e)
{
  std::vector<hep::rec_hdr> hdrs;
  e.load("a", hdrs);
  std::cerr << "Size of hdrs: " << hdrs.size() << "\n";
}

void
process_event(hepnos::Event const& e,
              Block* block,
              std::ostream* pdebug,
              std::vector<timing_record>& timingdata,
              hepnos::Prefetcher* prefetcher)
{
  if (pdebug)
    *pdebug << "Processing event: " << e.number() << '\n';
  timingdata.push_back({MPI_Wtime(), e.number(), Step::pre_create_records});
  auto records = ana::create_records<timing_record, hepnos::Prefetcher>(e, timingdata, prefetcher);
  timingdata.push_back(
    {MPI_Wtime(), records.size(), Step::post_create_records});
  if (pdebug)
    *pdebug << "Created records for event: " << e.number() << '\n';
  for (auto const& rec : records) {
    if (ana::good_slice(rec)) {
      auto const& cfg = block->configuration();
      auto const r_no = cfg.run_num;
      auto const sr_no = cfg.subrun_num;
      auto const e_no = e.number();
      auto const slc_no = rec.hdr.subevt;
      if (pdebug) {
        fmt::print(*pdebug,
                   "Good slice: {} {} {} {} {} {}\n",
                   r_no,
                   sr_no,
                   cfg.begin_eventnum,
                   cfg.max_events,
                   e_no,
                   slc_no);
      }
      SliceID sid{r_no, sr_no, e_no, slc_no};
      block->reduceData().push_back(sid);
    }
  }
  timingdata.push_back({MPI_Wtime(), static_cast<std::size_t>(block->gid()), Step::post_process_slices});
}

void
call_prefetcher(hepnos::Prefetcher& prefetcher, std::string label)
{
  prefetcher.fetchProduct<hep::rec_spill>(label);
  prefetcher.fetchProduct<hep::rec_hdr>(label);
  prefetcher.fetchProduct<hep::rec_slc>(label);
  prefetcher.fetchProduct<hep::rec_vtx>(label);
  prefetcher.fetchProduct<hep::rec_sel_remid>(label);
  prefetcher.fetchProduct<hep::rec_sel_cvnProd3Train>(label);
  prefetcher.fetchProduct<hep::rec_sel_cvn2017>(label);
  prefetcher.fetchProduct<hep::rec_sel_contain>(label);
  prefetcher.fetchProduct<hep::rec_energy_numu>(label);
  prefetcher.fetchProduct<hep::rec_vtx_elastic_fuzzyk>(label);
  prefetcher.fetchProduct<hep::rec_trk_cosmic>(label);
  prefetcher.fetchProduct<hep::rec_trk_kalman>(label);
  prefetcher.fetchProduct<hep::rec_vtx_elastic_fuzzyk_png>(label);
  prefetcher.fetchProduct<hep::rec_vtx_elastic_fuzzyk_png_shwlid>(label);
  prefetcher.fetchProduct<hep::rec_vtx_elastic_fuzzyk_png_cvnpart>(label);
  prefetcher.fetchProduct<hep::rec_trk_kalman_tracks>(label);
}

//in future use_prefetch should be replaced by something meaningful
// using an enum may be more clear at the call side
void
process_block(Block* block,
              diy::Master::ProxyWithLink const& /* cp */,
              int /* rank */,
              hepnos::DataStore& datastore,
              std::string const& dataset_name,
              std::ostream* pdebug,
              std::vector<timing_record>& timingdata,
              bool use_prefetch)
{
  hepnos::DataSet dataset = datastore.root()[dataset_name];
  std::unique_ptr<hepnos::Prefetcher> pprefetcher;
  if (use_prefetch) {
    pprefetcher = std::make_unique<hepnos::Prefetcher>(datastore);
    call_prefetcher(*pprefetcher, "a");
  }
  hepnos::Prefetcher* prefetcher = pprefetcher.get();
  
  auto const& config = block->configuration();
  timingdata.push_back(
    {MPI_Wtime(), config.subrun_num, Step::pre_process_block});
  auto r = dataset.createRun(config.run_num);
  auto sr = r.createSubRun(config.subrun_num);
  timingdata.reserve(config.max_events);
  hepnos::SubRun::const_iterator ievt =
    (use_prefetch ? sr.find(config.begin_eventnum, *prefetcher) :
                  sr.find(config.begin_eventnum));

  std::size_t nevts = 0;
  for (std::size_t i = 0; i != config.max_events && ievt != sr.end();
       ++i, ++ievt) {
    process_event(*ievt, block, pdebug, timingdata, prefetcher);
    nevts += 1;
  }
  timingdata.push_back({MPI_Wtime(), nevts, Step::post_process_block});
}

void get_tot_work(Block* b,                             // local block
                  const diy::Master::ProxyWithLink& cp) // communication proxy
{
  fmt::print(stderr,
             "[{}] Num slices passed = {}\n",
             cp.gid(),
             b->reduceData().slices.size());
}

template <size_t N>
std::array<char, N>
string_to_array(std::string str)
{
  if (str.size() > N) {
    throw std::runtime_error(fmt::format("Filename too long: {}\n", str));
  }
  size_t i = 0ULL;
  std::array<char, N> arr;
  for (auto const s : str) {
    arr[i] = s;
    ++i;
  }
  arr[i] = '\0';
  return arr;
}


int
work(int argc, char* argv[])
{
  diy::mpi::environment env(argc, argv);
  diy::mpi::communicator world;

  std::string hepnos_config;
  std::string dataset_name;
  size_t maxevents{0};
  int radix{0};
  std::string outdir;
  bool barrier{false};
  bool oldtarget{false};
  bool prefetch{false};
  bool distancedoubling{true}; //distance halving if false
  std::string debug_dir;
  std::array<char, 128> hepnos_config_a;
  std::array<char, 128> dataset_name_a;
  std::array<char, 128> outdir_a;
  std::array<char, 128> debug_dir_a;
  int status = 0;
  if (world.rank() == 0) {
    CLI::App app{"eventselection"};
    app
      .add_option(
        "-f,--config", hepnos_config, "HEPnOS client configuration file name")
      ->check(CLI::ExistingFile);
    app.add_option("-s,--dsname", dataset_name, "Dataset name")->required();
    app
      .add_option(
        "-m,--maxevents", maxevents, "Maximum number of events per block")
      ->required();
    app.add_option("-k, --radix", radix, "Radix of tree reduction")->required();
    auto* opp =
      app.add_option("-o,--outdir", outdir, "Output and timing directory");
    opp->check(CLI::ExistingDirectory);
    opp->required();
    app.add_flag("--barrier", barrier, "Enable barrier before reduction phase");
    app.add_flag("--oldtarget", oldtarget, "Use target distribution");
    app.add_flag("--prefetch", prefetch, "Use prefetch");
    app.add_flag("--distancehalving", distancedoubling, "Use distance halving instead of distance doubling");
    app.add_option("-d, --debug", debug_dir);
    // CLI11_PARSE(app, argc, argv);
    try {
      app.parse(argc, argv);
      diy::mpi::broadcast(world, status, 0);
    }
    catch (const CLI::ParseError& e) {
      status = 1;
      diy::mpi::broadcast(world, status, 0);
      return app.exit(e);
    }
    hepnos_config_a = string_to_array<128>(hepnos_config);
    dataset_name_a = string_to_array<128>(dataset_name);
    outdir_a = string_to_array<128>(outdir);
    debug_dir_a = string_to_array<128>(debug_dir);
  } else {
    diy::mpi::broadcast(world, status, 0);
    if (status == 1)
      return 1;
  }
  diy::mpi::broadcast(world, hepnos_config_a, 0);
  diy::mpi::broadcast(world, dataset_name_a, 0);
  diy::mpi::broadcast(world, maxevents, 0);
  diy::mpi::broadcast(world, outdir_a, 0);
  diy::mpi::broadcast(world, barrier, 0);
  diy::mpi::broadcast(world, oldtarget, 0);
  diy::mpi::broadcast(world, prefetch, 0);
  diy::mpi::broadcast(world, radix, 0);
  diy::mpi::broadcast(world, distancedoubling, 0);
  diy::mpi::broadcast(world, debug_dir_a, 0);
  hepnos_config = hepnos_config_a.data();
  dataset_name = dataset_name_a.data();
  outdir = outdir_a.data();
  debug_dir = debug_dir_a.data();

  std::ofstream timingfile(
    fmt::format(
      "{}/timing_{}_{}_{}.dat", outdir, world.rank(), world.size(), maxevents)
      .c_str());
  timingfile.precision(15);
  std::unique_ptr<std::ostream> debugfile;
  if (!debug_dir.empty()) {
    debugfile = std::make_unique<std::ofstream>(
      fmt::format("{}/debug_{}.out", debug_dir, world.rank()).c_str());
  }

  std::vector<timing_record> timingdata;
  timingdata.reserve(1000 * 1000); // a wise guess
  timingdata.push_back({MPI_Wtime(), 0, Step::start});

  // mem_blocks = -1 indicates that all blocks are in memory.
  int const mem_blocks = -1;

  int const threads = 1; // we use only one thread per rank
  int dim = 1;

  hepnos::DataStore datastore = hepnos::DataStore::connect(hepnos_config);
  auto const num_targets = datastore.numTargets(hepnos::ItemType::EVENT);
  // TODO: also make sure that world.size is an integer multiple of num_targets
  if (num_targets > world.size()) {
    if (world.rank() == 0)
      std::cerr << "Configuration has " << num_targets
                << " targets, and world.size is: " << world.size()
                << " please use at least use that many ranks\n";
    return 1;
  }

  // This already helps and is an improvement from
  // world.rank()%num_targets, but will create imbalance among targets
  // if nranks is not completely divisible by num_targets
  // In our runs, we currently are not running any such configuration.
  int target_id;
  if (oldtarget)
    target_id = world.rank() % num_targets;
  else
    target_id = world.rank() / (world.size() / num_targets);

  diy::mpi::communicator my_group = world.split(target_id, world.rank());

  hepnos::DataSet dataset = datastore.root()[dataset_name];
  timingdata.push_back({MPI_Wtime(), 0, Step::post_dataset});

  if (debugfile)
    *debugfile << "checking 1: " << num_targets << ", " << my_group.rank()
               << ", " << world.rank() << ", " << target_id << '\n';
  EventSelectionConfigs block_configurations;
  if (my_group.rank() == 0) {
    block_configurations =
      calculate_block_configs(dataset, maxevents, target_id, debugfile.get());
  }
  timingdata.push_back({MPI_Wtime(), 0, Step::post_block_configurations});
  // need to gather all the block configurations at every rank,
  // some ranks will have nothing to send since the targets they are connecting
  // to may return EventSet of size 0
  std::vector<EventSelectionConfigs> total_block_configurations;
  diy::mpi::all_gather(world, block_configurations, total_block_configurations);
  // diy::mpi::broadcast(world, block_configurations, 0);

  timingdata.push_back({MPI_Wtime(), 0, Step::post_broadcast});

  // -------- above this point is all initialization custom for this application

  // Determine [my_start, my_end), which is the range of block configurations to
  // be created by the current rank.
  EventSelectionConfigs bc;
  for (auto&& v : total_block_configurations) {
    for (auto& c : v)
      bc.push_back(c);
  }

  size_t const total_num_blocks = bc.size();
  if (debugfile)
    *debugfile << "checking 2: " << total_num_blocks << ", "
               << block_configurations.size() << ", " << my_group.rank() << ", "
               << my_group.size() << '\n';
  std::vector<EventSelectionConfig>::const_iterator my_start, my_end;
  std::tie(my_start, my_end) =
    split_configs<EventSelectionConfigs>(bc, world.rank(), world.size());

  // Debugging printout
  if (debugfile) {
    *debugfile << "Block configurations\n";
    for (auto i = my_start; i != my_end; ++i) {
      *debugfile << *i << '\n';
    }
  }
  // ----- starting here is a lot of standard boilerplate code for this kind of
  //       application.
  // diy initialization
  diy::FileStorage storage("./DIY.XXXXXX"); // used for blocks moved out of core
  diy::Master master(world, // master is the top-level diy object
                     threads,
                     mem_blocks,
                     &Block::create,
                     &Block::destroy,
                     &storage,
                     &Block::save,
                     &Block::load);

  // an object for adding new blocks to master
  AddBlock create_blocks_for_this_rank(
    master, my_start, my_end, &world, debugfile.get());

  //  -------

  // Our use of Bounds is terribly stunted. We seem only to need a
  // single domain, for the full set of configurations to be used.
  Bounds domain{1};
  domain.min[0] = 0;
  domain.max[0] = bc.size();

  
  //  diy::RoundRobinAssigner assigner(world.size(), total_num_blocks);
    diy::ContiguousAssigner assigner(world.size(), total_num_blocks);

  // decompose the domain into blocks
  // This is a DIY regular way to assign neighbors. You can do this manually.
  diy::RegularDecomposer<Bounds> decomposer(dim, domain, total_num_blocks);
  timingdata.push_back({MPI_Wtime(), 0, Step::pre_decompose});
  decomposer.decompose(world.rank(), assigner, create_blocks_for_this_rank);
  timingdata.push_back({MPI_Wtime(), 0, Step::post_decompose});

  // ----------- below is the processing for this application
  // threads active here
  // prefetcher

  auto execute_block =
    [&world, &debugfile, &timingdata, &datastore, &dataset_name, prefetch](
      Block* b, const diy::Master::ProxyWithLink& cp) {
      process_block(
        b, cp, world.rank(), datastore, dataset_name, debugfile.get(), timingdata, prefetch);
    };
  // before and after block, data is useless in this case, make it 0
  timingdata.push_back({MPI_Wtime(), 0, Step::pre_execute_block});
  master.foreach (execute_block);
  timingdata.push_back({MPI_Wtime(), 0, Step::post_execute_block});
  // this is MPI
  // merge-based reduction: create the partners that determine how groups are
  // formed in each round and then execute the reduction
  //int const k = 2; // the radix of the k-ary reduction tree
  // partners for merge over regular block grid
  timingdata.push_back({MPI_Wtime(), 0, Step::pre_create_partners});
  if (debugfile) *debugfile << "radix is :" << radix <<'\n';
  diy::RegularMergePartners partners(decomposer, radix, distancedoubling); 
  if (barrier) {
    if (debugfile)
      *debugfile << "Barrier on rank: " << world.rank() << '\n';
    world.barrier();
  }
  auto reduce_block = [&timingdata](Block* b,
                                    const diy::ReduceProxy& rp,
                                    const diy::RegularMergePartners& partners) {
    reduceData<Block>(b, rp, partners, timingdata);
  };
  timingdata.push_back({MPI_Wtime(), 0, Step::pre_reduction});
  diy::reduce(master, assigner, partners, reduce_block);
  timingdata.push_back({MPI_Wtime(), 0, Step::post_reduction});

  master.foreach ([&world, &debugfile, &outdir](
                    Block* b, const diy::Master::ProxyWithLink& cp) {
    if (world.rank() != 0)
      return;
    if (debugfile) {
      fmt::print(
        *debugfile, "Block got slices: {} \n", b->reduceData().slices.size());
    }
    if (cp.gid() == 0) {
      std::ofstream outfile(
        fmt::format("{}/out.dat", outdir, world.rank()).c_str());
      for (auto const& a : b->reduceData().slices) {
        fmt::print(outfile, "{} {} {} {}\n", a.run, a.subRun, a.event, a.slice);
      }
    }
  });
  timingdata.push_back({MPI_Wtime(), 0, Step::finish});
  for (auto const& r : timingdata)
    timingfile << r << '\n';
  return 0;
}

int
main(int argc, char* argv[])
{
  DisableDepManAll();
  try {
    return work(argc, argv);
  }
  catch (hepnos::Exception const& e) {
    cerr << e.what() << '\n';
  }
  catch (std::exception const& e) {
    cerr << e.what() << '\n';
  }
  catch (...) {
    cerr << "Unknown exception\n";
  }
  return 1;
}
