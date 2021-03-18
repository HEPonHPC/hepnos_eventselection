#include "TFile.h"
#include "TTree.h"

#include "StandardRecord/Proxy/SRProxy.h"
#include "StandardRecord/StandardRecord.h"
#include "StandardRecord/SRHeader.h"

void fill_vars(TString file, std::map<std::string,std::vector<float>> & vars) {

  std::vector<float> subevt;
  vars.insert(std::make_pair("subevt",subevt));
  std::vector<float> det;
  vars.insert(std::make_pair("det",det));
  std::vector<float> calE;
  vars.insert(std::make_pair("calE",calE));
  std::vector<float> nhit;
  vars.insert(std::make_pair("nhit",nhit));
  std::vector<float> nelastic;
  vars.insert(std::make_pair("nelastic",nelastic));
  std::vector<float> npng;
  vars.insert(std::make_pair("npng",npng));
  std::vector<float> longestidx;
  vars.insert(std::make_pair("longestidx",longestidx));
  std::vector<float> len;
  vars.insert(std::make_pair("len",len));
  std::vector<float> shwlid_calE;
  vars.insert(std::make_pair("shwlid_calE",shwlid_calE));
  std::vector<float> photonid;
  vars.insert(std::make_pair("photonid",photonid));
  std::vector<float> pizeroid;
  vars.insert(std::make_pair("pizeroid",pizeroid));
  std::vector<float> electronid;
  vars.insert(std::make_pair("electronid",electronid));
  std::vector<float> protonid;
  vars.insert(std::make_pair("protonid",protonid));
  std::vector<float> pionid;
  vars.insert(std::make_pair("pionid",pionid));
  std::vector<float> neutronid;
  vars.insert(std::make_pair("neutronid",neutronid));
  std::vector<float> otherid;
  vars.insert(std::make_pair("otherid",otherid));
  std::vector<float> muonid;
  vars.insert(std::make_pair("muonid",muonid));

  TFile *f = TFile::Open(file);
  TTree *recTree = (TTree*)f->Get("recTree");
  caf::StandardRecord* sr = 0;
  recTree->SetBranchAddress("rec", &sr);
  Int_t numberOfEntries = recTree->GetEntriesFast();
  for (Int_t event = 0; event < numberOfEntries; ++event)
    {
      recTree->GetEntry(event);
      vars.at("subevt").push_back(sr->hdr.subevt);
      vars.at("det").push_back(sr->hdr.det);
      vars.at("calE").push_back(sr->slc.calE);
      vars.at("nhit").push_back(sr->slc.nhit);

      int nelastic = sr->vtx.nelastic;
      vars.at("nelastic").push_back(nelastic);

      for(unsigned int i_elastic = 0; i_elastic < nelastic; ++i_elastic) {
        int npng = sr->vtx.elastic[i_elastic].fuzzyk.npng;
        vars.at("npng").push_back(npng);
        vars.at("longestidx").push_back(sr->vtx.elastic[i_elastic].fuzzyk.longestidx);

        for(unsigned int i_png = 0; i_png < npng; ++i_png) {
          vars.at("len").push_back(sr->vtx.elastic[i_elastic].fuzzyk.png[i_png].len);
          vars.at("shwlid_calE").push_back(sr->vtx.elastic[i_elastic].fuzzyk.png[i_png].shwlid.calE);
          vars.at("photonid").push_back(sr->vtx.elastic[i_elastic].fuzzyk.png[i_png].cvnpart.photonid);
          vars.at("pizeroid").push_back(sr->vtx.elastic[i_elastic].fuzzyk.png[i_png].cvnpart.pizeroid);
          vars.at("electronid").push_back(sr->vtx.elastic[i_elastic].fuzzyk.png[i_png].cvnpart.electronid);
          vars.at("protonid").push_back(sr->vtx.elastic[i_elastic].fuzzyk.png[i_png].cvnpart.protonid);
          vars.at("pionid").push_back(sr->vtx.elastic[i_elastic].fuzzyk.png[i_png].cvnpart.pionid);
          vars.at("neutronid").push_back(sr->vtx.elastic[i_elastic].fuzzyk.png[i_png].cvnpart.neutronid);
          vars.at("otherid").push_back(sr->vtx.elastic[i_elastic].fuzzyk.png[i_png].cvnpart.otherid);
          vars.at("muonid").push_back(sr->vtx.elastic[i_elastic].fuzzyk.png[i_png].cvnpart.muonid);
        }
      }
    }
  f->Close();
}

void caf_validation(TString fileA, TString fileB){

  std::map<std::string, vector<float>> vars_A;
  std::map<std::string, vector<float>> vars_B;

  fill_vars(fileA, vars_A);
  fill_vars(fileB, vars_B);

  std::cout << "Comparing variables :" << std::endl;
  for(auto & it : vars_A) {
    std::string a_var = it.first;
    std::cout << a_var << " : ";
    it.second == vars_B.at(a_var) ? std::cout << "OK" << std::endl :
      std::cout << " =/=" << std::endl;
  }

}
