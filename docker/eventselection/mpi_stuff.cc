#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <mutex>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string.h>
#include <sys/stat.h>
#include <thread>
#include <time.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

#include "mpi.h"

#include "dataformat/rec_hdr.hpp"
#include "dataformat/rec_vtx_elastic_fuzzyk_png2d.hpp"

// diy headers
#include <diy/assigner.hpp>
#include <diy/decomposition.hpp>
#include <diy/master.hpp>
#include <diy/mpi.hpp>
#include <diy/partners/merge.hpp>
#include <diy/reduce.hpp>
#include <diy/serialization.hpp>

// hepnos headers
#include <hepnos/DataSet.hpp>
#include <hepnos/DataStore.hpp>
#include <hepnos/Event.hpp>
#include <hepnos/Run.hpp>
#include <hepnos/RunSet.hpp>
#include <hepnos/SubRun.hpp>

using namespace std;

// ----------------

class Mpi {
public:
  Mpi(int argc, char* argv[]);
  ~Mpi();

  int
  rank() const
  {
    return rank_;
  }
  int
  np() const
  {
    return np_;
  }
  int
  hostid() const
  {
    return hostid_;
  }
  std::string
  name() const
  {
    return name_;
  }

  int
  rank_group() const
  {
    return rank_group_;
  }
  int
  np_group() const
  {
    return np_group_;
  }
  MPI_Comm
  comm_group() const
  {
    return comm_;
  }
  MPI_Comm
  comm() const
  {
    return MPI_COMM_WORLD;
  }

  void barrier();
  void barrier_group();

private:
  int np_;
  int rank_;
  int rank_group_;
  int np_group_;
  int hostid_;
  std::string name_;
  MPI_Comm comm_;
};

Mpi::Mpi(int argc, char* argv[])
{
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &np_);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank_);
  hostid_ = (unsigned int)(gethostid());

  unsigned int group_size = 1;

  hostid_ = (rank_) / group_size;
  cerr << "hostid_ = " << hostid_ << "\n";

  int name_sz = 0;
  char name_buf[MPI_MAX_PROCESSOR_NAME];
  MPI_Get_processor_name(name_buf, &name_sz);
  name_ = name_buf;

  MPI_Comm_split(MPI_COMM_WORLD, hostid_, rank_, &comm_);
  MPI_Comm_rank(comm_, &rank_group_);
  MPI_Comm_size(comm_, &np_group_);
}

Mpi::~Mpi()
{
  MPI_Finalize();
}

void
Mpi::barrier()
{
  MPI_Barrier(MPI_COMM_WORLD);
}

void
Mpi::barrier_group()
{
  MPI_Barrier(comm_);
}

// Extract run number or subrun number from a given file name
std::uint64_t
getNum(std::string fname, std::regex r)
{
  std::smatch match;
  if (std::regex_search(fname, match, r)) {
    std::string rs = match[0];
    return std::stoi(rs.erase(0, 2));
  }
  return -1;
}

// ----------------

template <typename T>
void process_file(std::string const& fname, hepnos::DataSet& dataset);
template <typename T>
std::pair<std::vector<T>, std::vector<uint64_t>> read_table(hid_t hdf_file);
template <typename T>
void process_table(hepnos::SubRun& sr, hid_t hdf_file);
template <typename T>
void process_current_batch(size_t b_idx,
                           size_t e_idx,
                           hepnos::Event& ev,
                           std::vector<T> const& table);

int
main(int argc, char* argv[])
{
  if (argc <= 2) {
    std::cout << "Please specify the path to the <hepnos-client.yaml> and name "
                 "of the input file with paths to the HDF5 files to read\n";
    return 1;
  }

  Mpi mpi(argc, argv);

  std::string const configfile(*(argv + 1));
  hepnos::DataStore datastore(configfile);
  hepnos::DataSet dataset = datastore.createDataSet("NOvA");

  string const pathsfile(*(argv + 2));
  std::ifstream filenames(pathsfile);
  if (!filenames) {
    std::cerr << "Unable to open file: " << pathsfile;
    return -1;
  }

  for (std::istream_iterator<string> it(filenames);
       it != std::istream_iterator<string>();
       ++it) {
    process_file<ABC::rec_hdr>(*it, dataset);
    process_file<ABC::rec_vtx_elastic_fuzzyk_png2d>(*it, dataset);
  }
  return 0;
}

// load all the relevant data into the dataset
template <typename T>
void
process_file(std::string const& fname, hepnos::DataSet& dataset)
{
  hid_t hdf_file = H5Fopen(fname.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
  auto r = dataset.createRun(getNum(fname, std::regex("(_r000)([0-9]{5})")));
  auto sr = r.createSubRun(getNum(fname, std::regex("(_s)([0-9]{2})")));
  process_table<T>(sr, hdf_file);
}

template <typename T>
std::pair<std::vector<T>, std::vector<uint64_t>>
read_table(hid_t hdf_file)
{
  // callback to read in a vector of table
  std::vector<T> table;
  std::vector<uint64_t> events;
  auto f = [&](uint64_t, uint64_t, uint64_t event, T const& obj) {
    table.push_back(obj);
    events.push_back(event);
  };
  T::from_hdf5(hdf_file, f);
  return {table, events};
}

template <typename T>
void
process_table(hepnos::SubRun& sr, hid_t hdf_file)
{

  std::vector<T> table;
  std::vector<uint64_t> events;
  std::tie(table, events) = read_table<T>(hdf_file);

  auto batch_begin = events.cbegin();
  auto checkeve = [](uint64_t i, uint64_t j) { return (i != j); };
  auto batch_end = std::adjacent_find(batch_begin, events.cend(), checkeve);

  while (batch_begin != events.cend()) {
    if (batch_end != events.cend())
      batch_end = batch_end + 1;
    auto ev = sr.createEvent(*batch_begin);
    size_t b_idx = batch_begin - events.cbegin();
    size_t e_idx = batch_end - events.cbegin();
    process_current_batch(b_idx, e_idx, ev, table);
    batch_begin = batch_end;
    batch_end = std::adjacent_find(batch_begin, events.cend(), checkeve);
  }
}

template <typename T>
void
process_current_batch(size_t b_idx,
                      size_t e_idx,
                      hepnos::Event& ev,
                      std::vector<T> const& table)
{
  typename std::vector<T>::const_iterator b = table.cbegin() + b_idx,
                                          e = table.cbegin() + e_idx;
  std::vector<T> tmp(b, e);
  ev.store("a", tmp);
}
