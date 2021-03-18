#ifndef CREATE_RECORDS_DEV_HPP
#define CREATE_RECORDS_DEV_HPP

#include <stdexcept>
//#include "StandardRecord/SRHeader.h"
//#include "StandardRecord/SRSlice.h"
//#include "StandardRecord/SRSpill.h"
//#include "StandardRecord/SRVertexBranch.h"
#include "StandardRecord/StandardRecord.h"
#include <hepnos.hpp>
#include <hepnos/Event.hpp>
#include <vector>

#include "timing_record.hpp"

namespace ana {
  template <typename T, typename C> 
  std::vector<caf::StandardRecord> 
  create_records(hepnos::Event const&,
                 std::vector<T>&,
                 C* p);
  bool good_slice(caf::StandardRecord const&);
}


#include "dataproducts.hpp"
#include "diy/fmt/format.h"

#include "StandardRecord/Proxy/CopyRecord.h"
#include "StandardRecord/Proxy/SRProxy.h"

#include "CAFAna/Core/Cut.h"
#include "CAFAna/Cuts/NueCuts2018.h"
#include "CAFAna/Cuts/NumuCuts2018.h"
#include "CAFAna/Cuts/NusCuts18.h"
#include "CAFAna/Vars/Vars.h"
//#include "diy/mpi.hpp"

bool
ana::good_slice(caf::StandardRecord const& rec)
{
  caf::SRProxy p(0, "");
  CopyRecord(rec, p);
  return kNumuCutND2018(&p);
};

void
fill_hdr(std::vector<caf::StandardRecord>& records,
         std::vector<hep::rec_hdr> const& rec_hdr)
{
  // size_t const nslices = rec_hdr.size();
  // for(size_t slice_id = 0; slice_id<nslices; ++slice_id) {
  for (auto const& hdr : rec_hdr) {
    auto slice_id = hdr.subevt - 1;
    auto& sr_hdr = records[slice_id];
    caf::Det_t fDet = (caf::Det_t)rec_hdr[slice_id].det;
    sr_hdr.hdr.det = fDet;
    sr_hdr.hdr.subevt = hdr.subevt;
    sr_hdr.hdr.run = hdr.run;
    sr_hdr.hdr.ismc = hdr.ismc;
  }
}

void
fill_slc(std::vector<caf::StandardRecord>& records,
         std::vector<hep::rec_slc> const& rec_slc)
{
  for (auto const& slc : rec_slc) {
    auto slice_id = slc.subevt - 1;
    auto& sr_slc = records[slice_id];
    //    sr_slc.slc.calE = rec_slc[slice_id].calE;
    //    sr_slc.slc.nhit = rec_slc[slice_id].nhit;
    //    sr_slc.slc.firstplane = rec_slc[slice_id].firstplane;
    //    sr_slc.slc.lastplane = rec_slc[slice_id].lastplane;
    //    sr_slc.slc.ncontplanes = rec_slc[slice_id].ncontplanes;
    //    sr_slc.slc.nhit = rec_slc[slice_id].nhit;
    sr_slc.slc.calE = slc.calE;
    sr_slc.slc.nhit = slc.nhit;
    sr_slc.slc.firstplane = slc.firstplane;
    sr_slc.slc.lastplane = slc.lastplane;
    sr_slc.slc.ncontplanes = slc.ncontplanes;
    sr_slc.slc.nhit = slc.nhit;
  }
}

void
fill_vtx(std::vector<caf::StandardRecord>& records,
         std::vector<hep::rec_vtx> const& rec_vtx)
{
  for (auto const& vtx : rec_vtx) {
    auto slice_id = vtx.subevt - 1;
    auto& sr_vtx = records[slice_id];
    sr_vtx.vtx.nelastic = rec_vtx[slice_id].nelastic;
    sr_vtx.vtx.elastic.resize(sr_vtx.vtx.nelastic);
  }
}

void
fill_spill(std::vector<caf::StandardRecord>& records,
           std::vector<hep::rec_spill> const& rec_spill)
{
  for (auto const& spill : rec_spill) {
    auto slice_id = spill.subevt - 1;
    auto& sr_spill = records[slice_id].spill;
    sr_spill.isRHC = spill.isRHC;
    sr_spill.isFHC = spill.isFHC;
  }
}

void
fill_fuzzyk(
  std::vector<caf::StandardRecord>& records,
  std::vector<hep::rec_vtx_elastic_fuzzyk> const& rec_vtx_elastic_fuzzyk)
{
  for (auto const& fuzzyk : rec_vtx_elastic_fuzzyk) {
    auto slice_id = fuzzyk.subevt - 1;
    auto elastic_id = fuzzyk.rec_vtx_elastic_idx;
    auto& sr_fuzzyk = records[slice_id].vtx.elastic[elastic_id];
    sr_fuzzyk.fuzzyk.npng = fuzzyk.npng;
    sr_fuzzyk.fuzzyk.longestidx = fuzzyk.longestidx;
    sr_fuzzyk.fuzzyk.nshwlid = fuzzyk.nshwlid;
    sr_fuzzyk.fuzzyk.png.resize(fuzzyk.npng);
  }
}

void
fill_png(std::vector<caf::StandardRecord>& records,
         std::vector<hep::rec_vtx_elastic_fuzzyk_png> const&
           rec_vtx_elastic_fuzzyk_png)
{
  for (auto const& prong : rec_vtx_elastic_fuzzyk_png) {
    auto slice_id = prong.subevt - 1;
    if (records[slice_id].vtx.nelastic == 0)
      continue;
    auto elastic_id = prong.rec_vtx_elastic_idx;
    auto prong_id = prong.rec_vtx_elastic_fuzzyk_png_idx;
    auto& sr_prong =
      records[slice_id].vtx.elastic[elastic_id].fuzzyk.png[prong_id];
    sr_prong.len = prong.len;
  }
}

void
fill_shwlid(std::vector<caf::StandardRecord>& records,
            std::vector<hep::rec_vtx_elastic_fuzzyk_png_shwlid> const&
              rec_vtx_elastic_fuzzyk_png_shwlid)
{
  for (auto const& shwlid : rec_vtx_elastic_fuzzyk_png_shwlid) {
    auto slice_id = shwlid.subevt - 1;
    auto elastic_id = shwlid.rec_vtx_elastic_idx;
    auto prong_id = shwlid.rec_vtx_elastic_fuzzyk_png_idx;
    auto& sr_shwlid =
      records[slice_id].vtx.elastic[elastic_id].fuzzyk.png[prong_id].shwlid;
    sr_shwlid.calE = shwlid.calE;
    sr_shwlid.start.x = shwlid.start_x;
    sr_shwlid.start.y = shwlid.start_y;
    sr_shwlid.start.z = shwlid.start_z;
    sr_shwlid.stop.x = shwlid.stop_x;
    sr_shwlid.stop.y = shwlid.stop_y;
    sr_shwlid.stop.z = shwlid.stop_z;
  }
}

void
fill_cvnpart(std::vector<caf::StandardRecord>& records,
             std::vector<hep::rec_vtx_elastic_fuzzyk_png_cvnpart> const&
               rec_vtx_elastic_fuzzyk_png_cvnpart)
{

  for (auto const& cvnpart : rec_vtx_elastic_fuzzyk_png_cvnpart) {
    auto slice_id = cvnpart.subevt - 1;
    auto elastic_id = cvnpart.rec_vtx_elastic_idx;
    auto prong_id = cvnpart.rec_vtx_elastic_fuzzyk_png_idx;
    auto& sr_cvnpart =
      records[slice_id].vtx.elastic[elastic_id].fuzzyk.png[prong_id].cvnpart;
    sr_cvnpart.photonid = cvnpart.photonid;
    sr_cvnpart.pizeroid = cvnpart.pizeroid;
    sr_cvnpart.electronid = cvnpart.electronid;
    sr_cvnpart.protonid = cvnpart.protonid;
    sr_cvnpart.pionid = cvnpart.pionid;
    sr_cvnpart.neutronid = cvnpart.neutronid;
    sr_cvnpart.otherid = cvnpart.otherid;
    sr_cvnpart.muonid = cvnpart.muonid;
  }
}

void
fill_trk_cosmic(std::vector<caf::StandardRecord>& records,
                std::vector<hep::rec_trk_cosmic> const& rec_trk_cosmic)
{
  for (auto const& trk_cosmic : rec_trk_cosmic) {
    auto slice_id = trk_cosmic.subevt - 1;
    auto& sr_trk_cosmic = records[slice_id].trk.cosmic;
    sr_trk_cosmic.ntracks = trk_cosmic.ntracks;
  }
}

void
fill_trk_kalman(std::vector<caf::StandardRecord>& records,
                std::vector<hep::rec_trk_kalman> const& rec_trk_kalman)
{
  for (auto const& trk_kalman : rec_trk_kalman) {
    auto slice_id = trk_kalman.subevt - 1;
    auto& sr_trk_kalman = records[slice_id].trk.kalman;
    sr_trk_kalman.ntracks = trk_kalman.ntracks;
    sr_trk_kalman.idxremid = trk_kalman.idxremid;
    sr_trk_kalman.tracks.resize(trk_kalman.ntracks);
  }
}

void
fill_trk_kalman_tracks(
  std::vector<caf::StandardRecord>& records,
  std::vector<hep::rec_trk_kalman_tracks> const& rec_trk_kalman_tracks)
{
  for (auto const& trk_kalman_tracks : rec_trk_kalman_tracks) {
    auto slice_id = trk_kalman_tracks.subevt - 1;
    auto track_idx = trk_kalman_tracks.rec_trk_kalman_tracks_idx;
    auto& sr_trk_kalman_tracks = records[slice_id].trk.kalman.tracks[track_idx];
    sr_trk_kalman_tracks.avedEdxlast10cm = trk_kalman_tracks.avedEdxlast10cm;
    sr_trk_kalman_tracks.avedEdxlast20cm = trk_kalman_tracks.avedEdxlast20cm;
    sr_trk_kalman_tracks.avedEdxlast30cm = trk_kalman_tracks.avedEdxlast30cm;
    sr_trk_kalman_tracks.avedEdxlast40cm = trk_kalman_tracks.avedEdxlast40cm;
    sr_trk_kalman_tracks.trkyposattrans = trk_kalman_tracks.trkyposattrans;
    sr_trk_kalman_tracks.trkbakcellnd = trk_kalman_tracks.trkbakcellnd;
    sr_trk_kalman_tracks.trkfwdcellnd = trk_kalman_tracks.trkfwdcellnd;
    sr_trk_kalman_tracks.dedxllh2017 = trk_kalman_tracks.dedxllh2017;
    sr_trk_kalman_tracks.scatllh2017 = trk_kalman_tracks.scatllh2017;
    sr_trk_kalman_tracks.rempid = trk_kalman_tracks.rempid;
    sr_trk_kalman_tracks.start.x = trk_kalman_tracks.start_x;
    sr_trk_kalman_tracks.start.y = trk_kalman_tracks.start_y;
    sr_trk_kalman_tracks.start.z = trk_kalman_tracks.start_z;
    sr_trk_kalman_tracks.stop.x = trk_kalman_tracks.stop_x;
    sr_trk_kalman_tracks.stop.y = trk_kalman_tracks.stop_y;
    sr_trk_kalman_tracks.stop.z = trk_kalman_tracks.stop_z;
    sr_trk_kalman_tracks.len = trk_kalman_tracks.len;
  }
}

void
fill_energy_numu(std::vector<caf::StandardRecord>& records,
                 std::vector<hep::rec_energy_numu> const& rec_energy_numu)
{
  for (auto const& energy_numu : rec_energy_numu) {
    auto slice_id = energy_numu.subevt - 1;
    auto& sr_energy_numu = records[slice_id].energy.numu;
    sr_energy_numu.hadcalE = energy_numu.hadcalE;
    sr_energy_numu.hadtrkE = energy_numu.hadtrkE;
    sr_energy_numu.ndtrkcalactE = energy_numu.ndtrkcalactE;
    sr_energy_numu.ndtrkcaltranE = energy_numu.ndtrkcaltranE;
    sr_energy_numu.ndtrklenact = energy_numu.ndtrklenact;
    sr_energy_numu.ndtrklencat = energy_numu.ndtrklencat;
    sr_energy_numu.trkccE = energy_numu.trkccE;
  }
}

void
fill_sel_contain(std::vector<caf::StandardRecord>& records,
                 std::vector<hep::rec_sel_contain> const& rec_sel_contain)
{
  for (auto const& sel_contain : rec_sel_contain) {
    auto slice_id = sel_contain.subevt - 1;
    auto& sr_sel_contain = records[slice_id].sel.contain;
    sr_sel_contain.kalbakcellnd = sel_contain.kalbakcellnd;
    sr_sel_contain.kalfwdcellnd = sel_contain.kalfwdcellnd;
    sr_sel_contain.kalyposattrans = sel_contain.kalyposattrans;
  }
}

void
fill_sel_cvn2017(std::vector<caf::StandardRecord>& records,
                 std::vector<hep::rec_sel_cvn2017> const& rec_sel_cvn2017)
{
  for (auto const& sel_cvn2017 : rec_sel_cvn2017) {
    auto slice_id = sel_cvn2017.subevt - 1;
    auto& sr_sel_cvn2017 = records[slice_id].sel.cvn2017;
    sr_sel_cvn2017.numuid = sel_cvn2017.numuid;
  }
}

void
fill_sel_cvnProd3Train(
  std::vector<caf::StandardRecord>& records,
  std::vector<hep::rec_sel_cvnProd3Train> const& rec_sel_cvnProd3Train)
{
  for (auto const& sel_cvnProd3Train : rec_sel_cvnProd3Train) {
    auto slice_id = sel_cvnProd3Train.subevt - 1;
    auto& sr_sel_cvnProd3Train = records[slice_id].sel.cvnProd3Train;
    sr_sel_cvnProd3Train.numuid = sel_cvnProd3Train.numuid;
  }
}

void
fill_sel_remid(std::vector<caf::StandardRecord>& records,
               std::vector<hep::rec_sel_remid> const& rec_sel_remid)
{
  for (auto const& sel_remid : rec_sel_remid) {
    auto slice_id = sel_remid.subevt - 1;
    auto& sr_sel_remid = records[slice_id].sel.remid;
    sr_sel_remid.pid = sel_remid.pid;
  }
}

template <typename TIMINGDATA, typename LOADER>
std::vector<TIMINGDATA>
event_load(hepnos::Event const& event,
           LOADER* p, //Cache object or prefetch 
           std::size_t& nbytes)
{
  std::vector<TIMINGDATA> recs;
  if (p)
    event.load(*p, "a", recs);
  else
    event.load("a", recs);
  nbytes += recs.size() * sizeof(TIMINGDATA);
  return recs;
}

template <typename TIMINGDATA, typename LOADER>
std::vector<caf::StandardRecord>
ana::create_records(hepnos::Event const& event,
                    std::vector<TIMINGDATA>& timingdata,
                    LOADER* p)
{
  std::size_t nbytes = 0;
  // enable prefetching
  auto rec_spill = event_load<hep::rec_spill>(event, p, nbytes);
  auto rec_hdr = event_load<hep::rec_hdr>(event, p, nbytes);
  auto rec_slc = event_load<hep::rec_slc>(event, p, nbytes);
  auto rec_vtx = event_load<hep::rec_vtx>(event, p, nbytes);
  auto rec_sel_remid = event_load<hep::rec_sel_remid>(event, p, nbytes);
  auto rec_sel_cvnProd3Train =
    event_load<hep::rec_sel_cvnProd3Train>(event, p, nbytes);
  auto rec_sel_cvn2017 = event_load<hep::rec_sel_cvn2017>(event, p, nbytes);
  auto rec_sel_contain = event_load<hep::rec_sel_contain>(event, p, nbytes);
  auto rec_energy_numu = event_load<hep::rec_energy_numu>(event, p, nbytes);
  auto rec_vtx_elastic_fuzzyk =
    event_load<hep::rec_vtx_elastic_fuzzyk>(event, p, nbytes);
  auto rec_trk_cosmic = event_load<hep::rec_trk_cosmic>(event, p, nbytes);
  auto rec_trk_kalman = event_load<hep::rec_trk_kalman>(event, p, nbytes);
  auto rec_vtx_elastic_fuzzyk_png =
    event_load<hep::rec_vtx_elastic_fuzzyk_png>(event, p, nbytes);
  auto rec_vtx_elastic_fuzzyk_png_shwlid =
    event_load<hep::rec_vtx_elastic_fuzzyk_png_shwlid>(event, p, nbytes);
  auto rec_vtx_elastic_fuzzyk_png_cvnpart =
    event_load<hep::rec_vtx_elastic_fuzzyk_png_cvnpart>(event, p, nbytes);
  auto rec_trk_kalman_tracks =
    event_load<hep::rec_trk_kalman_tracks>(event, p, nbytes);

 // timingdata.push_back({MPI_Wtime(), nbytes, Step::post_fill_records});

  size_t const nslices = rec_hdr.size();

  std::vector<caf::StandardRecord> records(nslices);
  // Top level
  fill_spill(records, rec_spill);
  fill_hdr(records, rec_hdr);
  fill_slc(records, rec_slc);
  fill_vtx(records, rec_vtx);

  // Lvl 1 nesting
  fill_sel_remid(records, rec_sel_remid);
  fill_sel_cvnProd3Train(records, rec_sel_cvnProd3Train);
  fill_sel_cvn2017(records, rec_sel_cvn2017);
  fill_sel_contain(records, rec_sel_contain);
  fill_energy_numu(records, rec_energy_numu);
  fill_fuzzyk(records, rec_vtx_elastic_fuzzyk);
  fill_trk_cosmic(records, rec_trk_cosmic);
  fill_trk_kalman(records, rec_trk_kalman);

  // Lvl 2 nesting
  fill_png(records, rec_vtx_elastic_fuzzyk_png);
  fill_shwlid(records, rec_vtx_elastic_fuzzyk_png_shwlid);
  fill_cvnpart(records, rec_vtx_elastic_fuzzyk_png_cvnpart);
  fill_trk_kalman_tracks(records, rec_trk_kalman_tracks);

  return records;
};
#endif
