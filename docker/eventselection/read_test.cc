#include <iostream>
#include <string>
#include <vector>

#include "dataformat/rec_vtx_elastic_fuzzyk_png2d.hpp"

int
main(int argc, char* argv[])
{

  std::vector<std::string> args(argv + 1, argv + argc);

  std::vector<ABC::rec_vtx_elastic_fuzzyk_png2d> prongs;

  auto f = [&prongs](unsigned run,
                     unsigned subrun,
                     unsigned event,
                     ABC::rec_vtx_elastic_fuzzyk_png2d& obj) {
    prongs.push_back(obj);
    std::cout << run << " " << subrun << " " << event << " " << obj.subevt
              << "\n";
  };

  for (auto const& arg : args) {
    ABC::rec_vtx_elastic_fuzzyk_png2d::from_hdf5(arg, f);
  }
  std::cout << prongs.size() << "\n";
  return 0;
}
