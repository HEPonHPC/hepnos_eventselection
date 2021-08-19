
#include <hepnos/DataSet.hpp>
#include <hepnos/DataStore.hpp>
#include <hepnos/Run.hpp>
#include <hepnos/RunSet.hpp>

#include <iostream>

int
main(int argc, char* argv[])
{
  if (argc < 3) {
    std::cerr << "Please specify a HEPnOS client configuration file name and "
                 "directory to the HDF5 files\n";
    return 1;
  }
  // MPI initialization
  //
  //
  std::string dirpath = argv[1];
  if (rank == 0) {
    hepnos::DataStore datastore(argv[2]);
  }
  // MPI Barrier
  //

  //  auto ds1 = datastore.createDataSet("Fermilab");
  //  auto ds2 = ds1.createDataSet("Epoch2");
  //  auto r = ds2.createRun(1000);
  //  auto sr1 = r.createSubRun(34);
  //  auto sr2 = r.createSubRun(35);
  //  auto sr3 = r.createSubRun(36);
  //
  //  sr1.createEvent(100);
  //  sr1.createEvent(101);
  //  sr1.createEvent(102);
  //
  //  sr2.createEvent(100);
  //  sr2.createEvent(101);
  //  sr2.createEvent(102);
  //
  //  sr3.createEvent(100);
  //  sr3.createEvent(101);
  //
  return 0;
}
