#include "CAFAna/Cuts/Cuts.h"
#include "CAFAna/Cuts/NumuCuts2018.h"
#include "CAFAna/Core/EventList.h"

void caf_cut() {

  const std::string fInput = "8files/neardet_r00011990_s02_t00_R19-02-23-miniprod5.i_v1_data.caf.root";

  MakeEventListFile(fInput,ana::kNumuCutND2018,"slices_list.txt",true);

}
