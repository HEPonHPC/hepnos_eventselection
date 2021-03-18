#include "../code/catch.hpp"
#include "TFile.h"
#include "TTree.h"
#include <iostream>
#include <string>
#include <vector>

#include "create_records_dev.hpp"
#include "timing_record.hpp"
#include <hepnos/DataSet.hpp>
#include <hepnos/DataStore.hpp>
#include <hepnos/Run.hpp>
#include <hepnos/RunSet.hpp>

int
main(int argc, char* argv[])
{
  TFile* output_file = new TFile("test.root", "RECREATE");
  TTree* rec_tree = new TTree("recTree", "records");
  caf::StandardRecord* dummy_rec = 0;
  rec_tree->Branch("rec", "caf::StandardRecord", &dummy_rec);

  std::vector<caf::StandardRecord> v_sr;

  int count_datastore = 0;
  int count_run = 0;
  int count_subrun = 0;
  int count_event = 0;
  hepnos::DataStore datastore("../client.yaml");
  std::vector<timing_record> td;
  for (auto it = datastore.begin(); it != datastore.end(); ++it) {
    if (count_datastore > 0)
      break;
    std::cout << "Dataset from iterator: " << it->fullname() << std::endl;
    for (auto const& r : it->runs()) {
      // if(count_run>0)
      //   break;
      std::cout << "Run Number: " << r.number() << std::endl;
      for (auto const& sr : r) {
        // if(count_subrun>0)
        //   break;
        std::cout << "SubRun Number: " << sr.number() << std::endl;
        for (auto const& e : sr) {
          // if(count_event>0)
          //   break;
          std::cout << "Event Number: " << e.number() << ", ";
          auto tmp_sr = ana::create_records(e, td);
          // Add run, subrun, event to each subevent (i.e. slice)
          v_sr.insert(v_sr.end(), tmp_sr.begin(), tmp_sr.end());
          ++count_event;
        }
        ++count_subrun;
      }
      ++count_run;
    }
    ++count_datastore;
  }

  for (auto& a_sr : v_sr) {
    caf::StandardRecord rec;
    caf::StandardRecord* prec = &rec;
    rec_tree->SetBranchAddress("rec", &prec);
    prec = &a_sr;
    rec_tree->Fill();
  }

  output_file->Write();
  output_file->Close();

  return 0;
}
