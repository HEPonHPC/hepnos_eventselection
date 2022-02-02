#include <mpi.h>

#include "DummyProduct.hpp"
#include "_test_macro_.hpp"
#include "create_records_dev.hpp"
#include "dataproducts.hpp"
#include "disable_depman.hpp"
#include <fstream>
#include <hepnos.hpp>
#include <iostream>
#include <random>
#include <regex>
#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>
#include <sstream>
#include <string>
#include <tclap/CmdLine.h>
#include <thallium.hpp>

#include "SliceID.hpp"
#include "SliceIDs.hpp"
#include "timing_record_for_bm.hpp"

static int g_size;
static int g_rank;
static std::string g_connection_file;
static std::string g_protocol;
static std::string g_output_dir;
static std::string g_input_dataset;
static std::string g_product_label;
static spdlog::level::level_enum g_logging_level;
static unsigned g_num_threads;
static std::vector<std::string> g_product_names;
static bool g_preload_products;
static std::pair<double, double> g_wait_range;
static std::unordered_map<std::string,
  std::function<void(hepnos::ParallelEventProcessor&)>>
  g_preload_fn;
  static std::mt19937 g_mte;
  static hepnos::ParallelEventProcessorOptions g_pep_options;
  static bool g_disable_stats;

  static void parse_arguments(int argc, char** argv);
  static std::pair<double, double> parse_wait_range(const std::string&);
  static std::string check_file_exists(const std::string& filename);
  static void prepare_preloading_functions();
  static void gather_results(int local_slice_count,
      SliceIDs const& good_slices,
      int g_rank,
      std::string g_output_dir);
static void run_benchmark();
template <typename Ostream>
static Ostream& operator<<(
    Ostream& os,
    const hepnos::ParallelEventProcessorStatistics& stats);

std::vector<timing_record_for_bm> timingdata; /* global timing data vector, accessible by all functions */
double ref_time;                              /* reference time */


  int
main(int argc, char** argv)
{
  int provided, required = MPI_THREAD_MULTIPLE;
  MPI_Init_thread(&argc, &argv, required, &provided);
  MPI_Comm_size(MPI_COMM_WORLD, &g_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &g_rank);
  MPI_Barrier(MPI_COMM_WORLD); /* ensure each rank has a consistent ref_time! */
  ref_time = MPI_Wtime();
  DisableDepManAll();

  timingdata.reserve(1000 * 1000); // a wise guess
  timingdata.push_back({MPI_Wtime()-ref_time, 0, Steps::start});
  std::stringstream str_format;
  str_format << "[" << std::setw(6) << std::setfill('0') << g_rank << "|"
    << g_size << "] [%H:%M:%S.%F] [%n] [%^%l%$] %v";
  spdlog::set_pattern(str_format.str());

  parse_arguments(argc, argv);

  spdlog::set_level(g_logging_level);

  if (provided != required && g_rank == 0) {
    spdlog::warn("MPI doesn't provider MPI_THREAD_MULTIPLE");
  }

  spdlog::trace("connection file: {}", g_connection_file);
  spdlog::trace("Output dir: {}", g_output_dir);
  spdlog::trace("input dataset: {}", g_input_dataset);
  spdlog::trace("product label: {}", g_product_label);
  spdlog::trace("num threads: {}", g_num_threads);
  spdlog::trace("product names: {}", g_product_names.size());
  spdlog::trace("wait range: {},{}", g_wait_range.first, g_wait_range.second);
  // time stamp: before checking preloading, data is 0 for no preload and 1 for preload
  timingdata.push_back({MPI_Wtime()-ref_time, static_cast<size_t>(g_preload_products), Steps::pre_check_preload});
  if (g_preload_products) {
    // time stamp before prepare preload function
    timingdata.push_back({MPI_Wtime()-ref_time, 0, Steps::pre_prepare_preload});
    prepare_preloading_functions();
    // time stamp after prepare prelaod
    timingdata.push_back({MPI_Wtime()-ref_time, 0, Steps::post_prepare_preload});
    if (g_rank == 0) {
      for (auto& p : g_product_names) {
        if (g_preload_fn.count(p) == 0) {
          spdlog::critical("Unknown product name {}", p);
          MPI_Abort(MPI_COMM_WORLD, -1);
          exit(-1);
        }
      }
    }
  }
  // time stamp: after checking preload, capture g_preload_fn.size()
  timingdata.push_back({MPI_Wtime()-ref_time, g_preload_fn.size(), Steps::post_check_preload});
  MPI_Barrier(MPI_COMM_WORLD);
  // time stamp: after Barrier
  timingdata.push_back({MPI_Wtime()-ref_time, 0, Steps::post_barrier});

  spdlog::trace("Initializing RNG");
  g_mte = std::mt19937(g_rank);
  // time stamp: before benchmark
  timingdata.push_back({MPI_Wtime()-ref_time, 0, Steps::pre_run_benchmark});
  std::ofstream timingfile(
      fmt::format(
        "{}/timing_{}_{}_{}.dat", g_output_dir, g_rank, g_size, g_num_threads)
      .c_str());
  run_benchmark();
  // time stamp: after benchmark
  timingdata.push_back({MPI_Wtime()-ref_time, 0, Steps::post_run_benchmark});
  MPI_Finalize();

  // Write timing data to file
  for (auto const &r : timingdata)
    timingfile << r << '\n';

  return 0;
}

  static void
parse_arguments(int argc, char** argv)
{
  try {
    TCLAP::CmdLine cmd("Benchmark HEPnOS Parallel Event Processor", ' ', "0.1");
    // mandatory arguments
    TCLAP::ValueArg<std::string> clientFile(
        "c", "connection", "JSON connection file for HEPnOS", true, "", "string");
    TCLAP::ValueArg<std::string> protocol(
        "p", "protocol", "Protocol to connect to HEPnOS", true, "", "string");
    TCLAP::ValueArg<std::string> outDir(
        "O", "out", "Output directory for timing and output", true, "", "string");
    TCLAP::ValueArg<std::string> dataSetName(
        "d",
        "dataset",
        "DataSet from which to load the data",
        true,
        "",
        "string");
    TCLAP::ValueArg<std::string> productLabel(
        "l", "label", "Label to use when storing products", true, "", "string");
    // optional arguments
    std::vector<std::string> allowed = {
      "trace", "debug", "info", "warning", "error", "critical", "off"};
    TCLAP::ValuesConstraint<std::string> allowedVals(allowed);
    TCLAP::ValueArg<std::string> loggingLevel(
        "v",
        "verbose",
        "Logging output type (info, debug, critical)",
        false,
        "info",
        &allowedVals);
    TCLAP::ValueArg<unsigned> numThreads(
        "t",
        "threads",
        "Number of threads to run processing work",
        false,
        0,
        "int");
    TCLAP::MultiArg<std::string> productNames(
        "n", "product-names", "Name of the products to load", false, "string");
    TCLAP::ValueArg<std::string> waitRange(
        "r",
        "wait-range",
        "Waiting time interval in seconds (e.g. 1.34,3.56)",
        false,
        "0,0",
        "x,y");
    TCLAP::ValueArg<unsigned> inputBatchSize(
        "i",
        "input-batch-size",
        "Input batch size for parallel event processor",
        false,
        16,
        "int");
    TCLAP::ValueArg<unsigned> outputBatchSize(
        "o",
        "output-batch-size",
        "Output batch size for parallel event processor",
        false,
        16,
        "int");
    TCLAP::ValueArg<unsigned> cacheSize(
        "s",
        "cache-size",
        "Prefetcher cache size for parallel event processor",
        false,
        std::numeric_limits<unsigned>::max(),
        "int");
    TCLAP::SwitchArg preloadProducts(
        "e", "preload", "Enable preloading products");
    TCLAP::SwitchArg disableStats(
        "", "disable-stats", "Disable statistics collection");

    cmd.add(clientFile);
    cmd.add(protocol);
    cmd.add(outDir);
    cmd.add(dataSetName);
    cmd.add(productLabel);
    cmd.add(loggingLevel);
    cmd.add(numThreads);
    cmd.add(productNames);
    cmd.add(waitRange);
    cmd.add(inputBatchSize);
    cmd.add(outputBatchSize);
    cmd.add(cacheSize);
    cmd.add(disableStats);
    cmd.add(preloadProducts);

    cmd.parse(argc, argv);

    g_connection_file = check_file_exists(clientFile.getValue());
    g_protocol = protocol.getValue();
    g_output_dir = outDir.getValue();
    g_input_dataset = dataSetName.getValue();
    g_product_label = productLabel.getValue();
    g_logging_level = spdlog::level::from_str(loggingLevel.getValue());
    g_num_threads = numThreads.getValue();
    g_product_names = productNames.getValue();
    g_preload_products = preloadProducts.getValue();
    g_wait_range = parse_wait_range(waitRange.getValue());
    g_pep_options.inputBatchSize = inputBatchSize.getValue();
    g_pep_options.outputBatchSize = outputBatchSize.getValue();
    g_pep_options.cacheSize = cacheSize.getValue();
    g_disable_stats = disableStats.getValue();
  }
  catch (TCLAP::ArgException& e) {
    if (g_rank == 0) {
      spdlog::critical("{} for command-line argument {}", e.error(), e.argId());
      MPI_Abort(MPI_COMM_WORLD, 1);
      exit(-1);
    }
  }
}

  static std::pair<double, double>
parse_wait_range(const std::string& s)
{
  std::pair<double, double> range = {0.0, 0.0};
  std::regex rgx(
      "^((0|([1-9][0-9]*))(\\.[0-9]+)?)(,((0|([1-9][0-9]*))(\\.[0-9]+)?))?$");
  // groups 1 and 6 will contain the two numbers
  std::smatch matches;

  if (std::regex_search(s, matches, rgx)) {
    range.first = atof(matches[1].str().c_str());
    if (matches[6].str().size() != 0) {
      range.second = atof(matches[6].str().c_str());
    } else {
      range.second = range.first;
    }
  } else {
    if (g_rank == 0) {
      spdlog::critical("Invalid wait range expression {} (should be \"x,y\" "
          "where x and y are floats)",
          s);
      MPI_Abort(MPI_COMM_WORLD, -1);
      exit(-1);
    }
  }
  if (range.second < range.first) {
    spdlog::critical("Invalid wait range expression {} ({} < {})",
        s,
        range.second,
        range.first);
    MPI_Abort(MPI_COMM_WORLD, -1);
    exit(-1);
  }

  return range;
}

  static std::string
check_file_exists(const std::string& filename)
{
  spdlog::trace("Checking if file {} exists", filename);
  std::ifstream ifs(filename);
  if (ifs.good())
    return filename;
  else {
    spdlog::critical("File {} does not exist", filename);
    MPI_Abort(MPI_COMM_WORLD, -1);
    exit(-1);
  }
  return "";
}

  static void
prepare_preloading_functions()
{
  spdlog::trace("Preparing functions for loading producs");
#define X(__class__)                                                           \
  spdlog::trace(                                                               \
      "Setting preloading function for product of type " #__class__);            \
  g_preload_fn[#__class__] = [](hepnos::ParallelEventProcessor& pep) {         \
    spdlog::trace("Will preload products of type " #__class__);                \
    pep.preload<std::vector<__class__>>(g_product_label);                      \
  };

  X(dummy_product)
    HEPNOS_FOREACH_NOVA_CLASS
#undef X
    spdlog::trace("Created preloading functions for {} product types",
        g_preload_fn.size());
}

  static void
simulate_processing(const hepnos::Event& ev,
    const hepnos::ProductCache& cache,
    std::vector<SliceIDs>& good_slices_per_thread,
    int thread_id)
{
  spdlog::trace("Loading products");

  std::vector<timing_record_for_bm> timingdata;
  /* std::cout << "Processing event: " << ev.number() << '\n'; */
  hepnos::ProductCache const* pcache = nullptr;
  if (g_preload_products)
    pcache = &cache;
  auto records = ana::create_records(ev, timingdata, pcache);
  /* std::cout << "After creating records event\n"; */
  for (auto const& rec : records) {
    /* std::cout << "Processing record: " << rec.hdr.subevt << '\n'; */
    if (ana::good_slice(rec)) {
      std::cout << "passed slice for event: " << ev.number() << ", "
        << rec.hdr.subevt << '\n';
      SliceID sid{ev.subrun().run().number(),
        ev.subrun().number(),
        ev.number(),
        rec.hdr.subevt};
      good_slices_per_thread[thread_id].push_back(sid);
    }
  }
}

  static void
gather_results(int local_slice_count,
    SliceIDs const& good_slices,
    int g_rank,
    std::string g_output_dir)
{
  if (g_rank != 0) {
    MPI_Gather(
        &local_slice_count, 1, MPI_INT, NULL, 0, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Gatherv(good_slices.runs.data(),
        local_slice_count,
        MPI_INT,
        NULL,
        NULL,
        NULL,
        MPI_INT,
        0,
        MPI_COMM_WORLD);
    MPI_Gatherv(good_slices.subRuns.data(),
        local_slice_count,
        MPI_INT,
        NULL,
        NULL,
        NULL,
        MPI_INT,
        0,
        MPI_COMM_WORLD);
    MPI_Gatherv(good_slices.events.data(),
        local_slice_count,
        MPI_INT,
        NULL,
        NULL,
        NULL,
        MPI_INT,
        0,
        MPI_COMM_WORLD);
    MPI_Gatherv(good_slices.slices.data(),
        local_slice_count,
        MPI_INT,
        NULL,
        NULL,
        NULL,
        MPI_INT,
        0,
        MPI_COMM_WORLD);
  } else {
    std::vector<int> slice_count(g_size);
    MPI_Gather(&local_slice_count,
        1,
        MPI_INT,
        slice_count.data(),
        1,
        MPI_INT,
        0,
        MPI_COMM_WORLD);
    const std::size_t total_length =
      std::accumulate(std::begin(slice_count), std::end(slice_count), 0);
    std::vector<int> runs(total_length);
    std::vector<int> subRuns(total_length);
    std::vector<int> events(total_length);
    std::vector<int> slices(total_length);

    std::vector<int> offsets(g_size);
    offsets[0] = 0;
    std::partial_sum(std::begin(slice_count),
        std::end(slice_count) - 1,
        std::begin(offsets) + 1);

    MPI_Gatherv(good_slices.runs.data(),
        local_slice_count,
        MPI_INT,
        runs.data(),
        slice_count.data(),
        offsets.data(),
        MPI_INT,
        0,
        MPI_COMM_WORLD);
    MPI_Gatherv(good_slices.subRuns.data(),
        local_slice_count,
        MPI_INT,
        subRuns.data(),
        slice_count.data(),
        offsets.data(),
        MPI_INT,
        0,
        MPI_COMM_WORLD);
    MPI_Gatherv(good_slices.events.data(),
        local_slice_count,
        MPI_INT,
        events.data(),
        slice_count.data(),
        offsets.data(),
        MPI_INT,
        0,
        MPI_COMM_WORLD);
    MPI_Gatherv(good_slices.slices.data(),
        local_slice_count,
        MPI_INT,
        slices.data(),
        slice_count.data(),
        offsets.data(),
        MPI_INT,
        0,
        MPI_COMM_WORLD);

    // write output
    std::ofstream outfile(
        fmt::format("{}/out.dat", g_output_dir, g_rank).c_str());
    SliceIDs sids{runs, subRuns, events, slices};
    outfile << sids;
  }
}
  static void
run_benchmark()
{

  double t_start, t_end;
  hepnos::DataStore datastore;
  try {
    spdlog::trace("Connecting to HEPnOS using file {}", g_connection_file);
    // time stamp before connect
    timingdata.push_back({MPI_Wtime()-ref_time, 0, Steps::pre_connect_datastore});
    datastore = hepnos::DataStore::connect(g_protocol, g_connection_file);
    // time stamp after connect
    timingdata.push_back({MPI_Wtime()-ref_time, 0, Steps::post_connect_datastore});
  }
  catch (const hepnos::Exception& ex) {
    spdlog::critical("Could not connect to HEPnOS service: {}", ex.what());
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  {

    spdlog::trace("Creating AsyncEngine with {} threads", g_num_threads);
    // time stamp before async intiialization, capture num threads
    timingdata.push_back({MPI_Wtime()-ref_time, 0, Steps::pre_init_asyncengine});
    hepnos::AsyncEngine async(datastore, g_num_threads);
    // time stamp after async
    timingdata.push_back({MPI_Wtime()-ref_time, 0, Steps::post_init_asyncengine});

    spdlog::trace("Creating ParallelEventProcessor");
    // time stamp before PEP intiialization, capture g-pep_options
    timingdata.push_back({MPI_Wtime()-ref_time, 0, Steps::pre_init_pep});
    hepnos::ParallelEventProcessor pep(async, MPI_COMM_WORLD, g_pep_options);
    // time stamp after PEP intiialization
    timingdata.push_back({MPI_Wtime()-ref_time, 0, Steps::post_init_pep});

    if (g_preload_products) {
      spdlog::trace("Setting preload flags");
      for (auto& p : g_product_names) {
        // time stamp before preload, store a 32 bit checksum of the string name:92
        timingdata.push_back({MPI_Wtime()-ref_time, 0, Steps::pre_preload, p});
        g_preload_fn[p](pep);
        // time stamp after preload
        timingdata.push_back({MPI_Wtime()-ref_time, 0, Steps::post_preload, p});
        // make another global hash map, value is the counter
      }
    }
    spdlog::trace("Loading dataset");
    hepnos::DataSet dataset;
    try {

      // time stamp before read dataset
      timingdata.push_back({MPI_Wtime()-ref_time, 0, Steps::pre_read_dataset});
      dataset = datastore.root()[g_input_dataset];
      // time stamp after read dataset
      timingdata.push_back({MPI_Wtime()-ref_time, 0, Steps::post_read_dataset});
    }
    catch (...) {
    }
    if (!dataset.valid() && g_rank == 0) {
      spdlog::critical("Invalid dataset {}", g_input_dataset);
      MPI_Abort(MPI_COMM_WORLD, -1);
      exit(-1);
    }
    // time stamp before barrier after dataset
    timingdata.push_back({MPI_Wtime()-ref_time, 0, Steps::pre_post_read_barrier});
    MPI_Barrier(MPI_COMM_WORLD);
    // time stamp after barrier after dataset
    timingdata.push_back({MPI_Wtime()-ref_time, 0, Steps::post_post_read_barrier});

    spdlog::trace("Calling processing function on dataset {}", g_input_dataset);

    hepnos::ParallelEventProcessorStatistics stats;
    hepnos::ParallelEventProcessorStatistics* stats_ptr = &stats;
    if (g_disable_stats)
      stats_ptr = nullptr;

    MPI_Barrier(MPI_COMM_WORLD);

    using result_buffer = std::vector<SliceIDs>;
    result_buffer good_slices_per_thread{
      static_cast<std::size_t>(g_num_threads)};
    for (auto& x : good_slices_per_thread)
      x.reserve(1000);
    t_start = MPI_Wtime();
    // time stamp pre pep process
    timingdata.push_back({MPI_Wtime()-ref_time, 0, Steps::pre_pep_process});
    pep.process(
        dataset,
        [&good_slices_per_thread](const hepnos::Event& ev,
          const hepnos::ProductCache& cache) {
        auto subrun = ev.subrun();
        auto run = subrun.run();
        spdlog::trace("Processing event {} from subrun {} from run {}",
            ev.number(),
            subrun.number(),
            run.number());
        auto thread_id = thallium::xstream::self().get_rank() - 1;
        simulate_processing(ev, cache, good_slices_per_thread, thread_id);
        },
        stats_ptr);
    // time stamp post pep process
    timingdata.push_back({MPI_Wtime()-ref_time, 0, Steps::post_pep_process});

    timingdata.push_back({MPI_Wtime()-ref_time, 0, Steps::pre_pep_process_barrier});
    MPI_Barrier(MPI_COMM_WORLD);
    // time stamp post pep process barrier
    timingdata.push_back({MPI_Wtime()-ref_time, 0, Steps::post_pep_process_barrier});

    t_end = MPI_Wtime();

    SliceIDs good_slices = std::accumulate(
        good_slices_per_thread.begin(), good_slices_per_thread.end(), SliceIDs{});

    // Now gather results from all MPI ranks and write to out file
    int local_slice_count = good_slices.size();

    // Rank 0 will first get all the counts of passed slices in slice_count,
    // then we need
    // to gather all the slide IDs, so we need 4 MPI_Gatherv -- one for run
    // numbers, one for subrun numbers, one for event numbers and one for slice
    // numbers, and then we can write the output to a file!
    //
    // time stamp pre gather results
    gather_results(local_slice_count, good_slices, g_rank, g_output_dir);
    // time stamp post gather results
    if (!g_disable_stats)
      spdlog::info("Statistics: {}", stats);
  }

  MPI_Barrier(MPI_COMM_WORLD);
  // time stamp post gather results barrier
  if (g_rank == 0)
    spdlog::info("Benchmark completed in {} seconds", t_end - t_start);
}

template <typename Ostream>
  static Ostream&
operator<<(Ostream& os, const hepnos::ParallelEventProcessorStatistics& stats)
{
  os << "{ \"total_events_processed\" : " << stats.total_events_processed << ","
    << " \"local_events_processed\" : " << stats.local_events_processed << ","
    << " \"total_time\" : " << stats.total_time << ","
    << " \"acc_event_processing_time\" : " << stats.acc_event_processing_time
    << ","
    << " \"acc_product_loading_time\" : " << stats.acc_product_loading_time
    << ","
    << " \"processing_time_stats\" : " << stats.processing_time_stats << ","
    << " \"waiting_time_stats\" : " << stats.waiting_time_stats << "}";
  return os;
}
