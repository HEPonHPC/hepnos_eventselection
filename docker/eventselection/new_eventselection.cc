#include <algorithm>
#include <iostream>
#include <iterator>
#include <numeric>
#include <mutex>

#include <mpi.h>
#include <thallium.hpp>

#include "create_records_dev.hpp"
#include "disable_depman.hpp"
#include "mpicpp.hpp"
#include "timing_record_for_mpi.hpp"
#include <hepnos.hpp>
#include <hepnos/DataSet.hpp>
#include <hepnos/DataStore.hpp>

#include "CLI11.hpp"
#include "SliceID.hpp"
#include "SliceIDs.hpp"
#include "dataproducts.hpp"
#include "diy/fmt/format.h"

using namespace std;

template <size_t N> std::array<char, N> string_to_array(std::string str) {
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

void preload(hepnos::ParallelEventProcessor &p, const std::string &label) {
  p.preload<hep::rec_spill>(label);
  p.preload<hep::rec_hdr>(label);
  p.preload<hep::rec_slc>(label);
  p.preload<hep::rec_vtx>(label);
  p.preload<hep::rec_sel_remid>(label);
  p.preload<hep::rec_sel_cvnProd3Train>(label);
  p.preload<hep::rec_sel_cvn2017>(label);
  p.preload<hep::rec_sel_contain>(label);
  p.preload<hep::rec_energy_numu>(label);
  p.preload<hep::rec_vtx_elastic_fuzzyk>(label);
  p.preload<hep::rec_trk_cosmic>(label);
  p.preload<hep::rec_trk_kalman>(label);
  p.preload<hep::rec_vtx_elastic_fuzzyk_png>(label);
  p.preload<hep::rec_vtx_elastic_fuzzyk_png_shwlid>(label);
  p.preload<hep::rec_vtx_elastic_fuzzyk_png_cvnpart>(label);
  p.preload<hep::rec_trk_kalman_tracks>(label);
}

int work(int argc, char *argv[], Mpi &world) {
  std::vector<timing_record_for_mpi> timingdata;
  timingdata.reserve(1000 * 1000); // a wise guess
  timingdata.push_back({MPI_Wtime(), 0, Steps::start});
  auto my_rank = world.rank();
  auto nranks = world.np();
  std::string hepnos_config;
  std::string dataset_name;
  std::string outdir;
  std::string debug_dir;
  std::array<char, 128> hepnos_config_a;
  std::array<char, 128> dataset_name_a;
  std::array<char, 128> outdir_a;
  std::array<char, 128> debug_dir_a;
  int status = 0;
  int num_threads = 0;
  if (my_rank == 0) {
    CLI::App app{"eventselection"};
    app.add_option("-t,--threads", num_threads, "Number of threads")
        ->required();
    app.add_option("-f,--config", hepnos_config,
                   "HEPnOS client configuration file name")
        ->check(CLI::ExistingFile);
    app.add_option("-s,--dsname", dataset_name, "Dataset name")->required();
    auto *opp =
        app.add_option("-o,--outdir", outdir, "Output and timing directory");
    opp->check(CLI::ExistingDirectory);
    opp->required();
    app.add_option("-d, --debug", debug_dir);
    try {
      app.parse(argc, argv);
      world.broadcast(&status, 0);
    } catch (const CLI::ParseError &e) {
      status = 1;
      world.broadcast(&status, 0);
      return app.exit(e);
    }
    hepnos_config_a = string_to_array<128>(hepnos_config);
    dataset_name_a = string_to_array<128>(dataset_name);
    outdir_a = string_to_array<128>(outdir);
    debug_dir_a = string_to_array<128>(debug_dir);
  } else {
    world.broadcast(&status, 0);
    if (status == 1)
      return 1;
  }
  world.broadcast(&num_threads, 0);
  world.broadcast(&hepnos_config_a, 0);
  world.broadcast(&dataset_name_a, 0);
  world.broadcast(&outdir_a, 0);
  world.broadcast(&debug_dir_a, 0);

  hepnos_config = hepnos_config_a.data();
  dataset_name = dataset_name_a.data();
  outdir = outdir_a.data();
  std::ofstream timingfile(fmt::format("{}/timing_{}_{}_{}.dat", outdir,
                                       my_rank, nranks, num_threads)
                               .c_str());
  timingfile.precision(15);
  debug_dir = debug_dir_a.data();
  std::unique_ptr<std::ostream> debugfile;
  if (!debug_dir.empty()) {
    debugfile = std::make_unique<std::ofstream>(
        fmt::format("{}/debug_{}.out", debug_dir, my_rank).c_str());
  }
  if (debugfile) {
    *debugfile << "Before connecting to  hepnos " << hepnos_config << std::endl;
  }
  timingdata.push_back({MPI_Wtime(), 0, Steps::pre_connect_datastore});
  hepnos::DataStore datastore = hepnos::DataStore::connect(hepnos_config);
  if (debugfile) {
    *debugfile << "Connected to hepnos" << std::endl;
  }

  timingdata.push_back({MPI_Wtime(), 0, Steps::pre_create_dataset});
  hepnos::DataSet dataset = datastore.root()[dataset_name];
  if (debugfile) {
    *debugfile << "Dataset created" << std::endl;
  }
  timingdata.push_back({MPI_Wtime(), 0, Steps::pre_create_asyncengine});
  hepnos::ParallelEventProcessorStatistics stats;
  hepnos::AsyncEngine async(datastore, num_threads);

  timingdata.push_back(
      {MPI_Wtime(), 0, Steps::pre_create_paralleleventprocessor});
  hepnos::ParallelEventProcessor parallel_processor(async, MPI_COMM_WORLD);
  if (debugfile) {
    *debugfile << "Created Parallel Event Processor" << std::endl;
  }
  timingdata.push_back({MPI_Wtime(), 0, Steps::pre_preload});
  preload(parallel_processor, "a");
  timingdata.push_back({MPI_Wtime(), 0, Steps::post_preload});
  if (debugfile) {
    *debugfile << "After preload" << std::endl;
  }

  using result_buffer = std::vector<SliceIDs>;
  result_buffer good_slices_per_thread{static_cast<std::size_t>(num_threads)};
  for (auto &x : good_slices_per_thread)
    x.reserve(1000);
  thallium::mutex debugfile_mutex;
  auto dowork = [world, &good_slices_per_thread, &timingdata,
                 &debugfile, &debugfile_mutex](const hepnos::Event &ev,
                             const hepnos::ProductCache &cache) {
    // adjusting thread_id to start from 0
    auto thread_id = thallium::xstream::self().get_rank() - 1;
    if (debugfile) {
      std::lock_guard<thallium::mutex> lock(debugfile_mutex);
      *debugfile << "Thread " << thread_id << " Before create_records" << std::endl;
    }
    hepnos::ProductCache const *ccache = nullptr;
    auto records = ana::create_records(ev, timingdata, ccache);
    if (debugfile) {
        std::lock_guard<thallium::mutex> lock(debugfile_mutex);
        *debugfile << "Thread " << thread_id << " processing event " << ev.number() << std::endl;
    }
    // loop over records, and call good_slice
    for (auto const &rec : records) {
      if (ana::good_slice(rec)) {
        SliceID sid{ev.subrun().run().number(), ev.subrun().number(),
                    ev.number(), rec.hdr.subevt};
        good_slices_per_thread[thread_id].push_back(sid);
      }
    }

    if ((thread_id == 0) && debugfile) {
        *debugfile << "Thread " << thread_id << " Done processing event " << ev.number() << std::endl;
    }
  };

  timingdata.push_back({MPI_Wtime(), 0, Steps::pre_process});
  parallel_processor.process(dataset, dowork, &stats);
  timingdata.push_back({MPI_Wtime(), 0, Steps::post_process});
  if (debugfile) {
    *debugfile << "Done with parallel_processor.process" << std::endl;
  }
  SliceIDs good_slices = std::accumulate(
      good_slices_per_thread.begin(), good_slices_per_thread.end(), SliceIDs{});
  if (debugfile) {
    *debugfile << "Before reduction" << std::endl;
  }
  // Now MPI reduction - only 1 thread,
  // First everyone send the number of slices passed to rank 0,
  // rank 0 will then assemble a buffer to gather all the passed slices in
  // only rank 0 will have meaningful slice_count, for other ranks this argument
  // is not used.
  timingdata.push_back({MPI_Wtime(), 0, Steps::pre_reduce_results});
  std::vector<int> slice_count{nranks};
  int local_slice_count = good_slices.size();
  MPI_Gather(&local_slice_count, 1, MPI_INT, slice_count.data(), 1, MPI_INT, 0,
             MPI_COMM_WORLD);
  // Now rank 0 has all the counts of passed slices in slice_count, now we need
  // to gather all the slide IDs, so we need 4 MPI_Gatherv -- one for run
  // numbers, one for subrun numbers, one for event numbers and one for slice
  // numbers, and then we can write the output to a file!
  //
  const std::size_t total_length =
      std::accumulate(std::begin(slice_count), std::end(slice_count), 0);
  std::vector<int> offsets{nranks};
  offsets[0] = 0;
  std::partial_sum(std::begin(slice_count), std::end(slice_count) - 1,
                   std::begin(offsets) + 1);
  if (my_rank != 0) {
    if (debugfile) {
      *debugfile << "Gatherv for runs, subruns, etc: {}" << std::endl;
    }
    MPI_Gatherv(good_slices.runs.data(), local_slice_count, MPI_INT, NULL,
                slice_count.data(), offsets.data(), MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Gatherv(good_slices.subRuns.data(), local_slice_count, MPI_INT, nullptr,
                slice_count.data(), offsets.data(), MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Gatherv(good_slices.events.data(), local_slice_count, MPI_INT, nullptr,
                slice_count.data(), offsets.data(), MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Gatherv(good_slices.slices.data(), local_slice_count, MPI_INT, nullptr,
                slice_count.data(), offsets.data(), MPI_INT, 0, MPI_COMM_WORLD);
    timingdata.push_back({MPI_Wtime(), 0, Steps::post_reduce_results});
  } else {
    std::vector<int> runs(total_length);
    std::vector<int> subRuns(total_length);
    std::vector<int> events(total_length);
    std::vector<int> slices(total_length);

    if (debugfile) {
      *debugfile << "Gatherv for runs, subruns, etc" << std::endl;
    }
    MPI_Gatherv(good_slices.runs.data(), local_slice_count, MPI_INT,
                runs.data(), slice_count.data(), offsets.data(), MPI_INT, 0,
                MPI_COMM_WORLD);
    MPI_Gatherv(good_slices.subRuns.data(), local_slice_count, MPI_INT,
                subRuns.data(), slice_count.data(), offsets.data(), MPI_INT, 0,
                MPI_COMM_WORLD);
   
   
   MPI_Gatherv(good_slices.events.data(), local_slice_count, MPI_INT,
                events.data(), slice_count.data(), offsets.data(), MPI_INT, 0,
                MPI_COMM_WORLD);
    MPI_Gatherv(good_slices.slices.data(), local_slice_count, MPI_INT,
                slices.data(), slice_count.data(), offsets.data(), MPI_INT, 0,
                MPI_COMM_WORLD);

    timingdata.push_back({MPI_Wtime(), 0, Steps::post_reduce_results});

    // write output
    std::ofstream outfile(
        fmt::format("{}/out.dat", outdir, world.rank()).c_str());
    SliceIDs sids{runs, subRuns, events, slices};
    outfile << sids;
  }
  if (debugfile) {
    *debugfile << "After reduction" << std::endl;
  }

  timingdata.push_back({MPI_Wtime(), 0, Steps::finish});
  for (auto const &r : timingdata)
    timingfile << r << '\n';
  return 0;
}

int main(int argc, char *argv[]) {
  int rc = 1;
  Mpi world(argc, argv);
  DisableDepManAll();
  try {
    rc = work(argc, argv, world);
  } catch (hepnos::Exception const &e) {
    cerr << e.what() << '\n';
  } catch (std::exception const &e) {
    cerr << e.what() << '\n';
  } catch (...) {
    cerr << "Unknown exception\n";
  }
  MPI_Finalize();
  return rc;
}
