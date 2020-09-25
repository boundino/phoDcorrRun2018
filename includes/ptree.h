#ifndef __PHOD_PTREE__
#define __PHOD_PTREE__

#include <TTree.h>

#include <vector>
#include <map>
#include <string>
#include <type_traits>

#include "param.h"

namespace phoD
{
  class ptree
  {
  public:
    ptree(TTree* nt);
    TTree* nt() { return nt_; }
    float val(std::string br, int j) { return (*bvf_[br])[j]; }
    bool status(std::string br) { return bvs_[br]; }
    int GetEntries() { return nt_->GetEntries(); }
    void GetEntry(int i) { nt_->GetEntry(i); }

    int nPho() { return nPho_; }
    int nEle() { return nEle_; }
    int nMC() { return nMC_; }
    bool isMC() { return isMC_; }

    bool presel(int j);
    bool sel_hem(int j, bool heavyion);
    bool sel_see(int j);
    bool sel_iso(int j, bool gen_iso);
    bool sel(int j, bool heavyion, bool gen_iso) { return (sel_hem(j, heavyion) && sel_see(j) && sel_iso(j, gen_iso)); }

  private:
    TTree* nt_;
    void setbranchaddress();
    int nPho_, nEle_, nMC_;
    bool isMC_;
    std::vector<std::string> tbvf_ = {
      "eleIP3D",
      "elePt",
      "eleEta",
      "elePhi",
      "eleHoverEBc",
      "eleEoverPInv",
      "eledPhiAtVtx",
      "eledEtaSeedAtVtx",
      "eleSigmaIEtaIEta_2012",
      "eleMissHits",
      "phoE",
      "phoEt",
      "phoEta",
      "phoPhi",
      "phoSCEta",
      "phoSCPhi",
      "phoHoverE",
      "phoSigmaIEtaIEta_2012",
      "pho_ecalClusterIsoR3",
      "pho_hcalRechitIsoR3",
      "pho_trackIsoR3PtCut20",
      "pho_genMatchedIndex",
      "mcPID",
      "mcStatus",
      "mcPt",
      "mcEta",
      "mcPhi",
      "mcEt",
      "mcCalIsoDR04",
    };
    std::map<std::string, std::vector<float>*> bvf_;
    std::map<std::string, bool> bvs_; // if the branch is valid
  };
}

phoD::ptree::ptree(TTree* nt) : nt_(nt)
{
  std::cout<<"\e[32;1m -- "<<__PRETTY_FUNCTION__<<"\e[0m"<<std::endl;

  isMC_ = false;

  nt_->SetBranchStatus("*", 0);
  nt_->SetBranchStatus("nEle", 1);
  nt_->SetBranchStatus("nPho", 1);
  if(nt_->FindBranch("nMC")) { nt_->SetBranchStatus("nPho", 1); isMC_ = true; }

  for(auto& b : tbvf_) { bvs_[b] = false;
  if(nt_->FindBranch(b.c_str())) { nt_->SetBranchStatus(b.c_str(), 1); bvs_[b] = true; } }
  for(auto& b : tbvf_) bvf_[b] = 0;
  setbranchaddress();
}

void phoD::ptree::setbranchaddress()
{
  nt_->SetBranchAddress("nPho", &nPho_);
  nt_->SetBranchAddress("nEle", &nEle_);
  if(nt_->FindBranch("nMC")) { nt_->SetBranchAddress("nMC", &nMC_); }
  for(auto& b : tbvf_) { if(bvs_[b]) { nt_->SetBranchAddress(b.c_str(), bvf_[b]); } }
}

bool phoD::ptree::presel(int j)
{
  bool basic = (*bvf_["phoEt"])[j] > photon_pt_min_ && std::abs((*bvf_["phoSCEta"])[j]) < photon_eta_abs_ && (*bvf_["phoHoverE"])[j] <= hovere_max_;
  return basic;
}

bool phoD::ptree::sel_hem(int j, bool heavyion)
{
  bool within_hem_failure_region = ((*bvf_["phoSCEta"])[j] < -1.3 && (*bvf_["phoSCPhi"])[j] < -0.87 && (*bvf_["phoSCPhi"])[j] > -1.57);
  return !(within_hem_failure_region && heavyion);
}

bool phoD::ptree::sel_see(int j)
{
  bool see = (*bvf_["phoSigmaIEtaIEta_2012"])[j] <= see_max_ && (*bvf_["phoSigmaIEtaIEta_2012"])[j] >= see_min_;
  return see;
}

bool phoD::ptree::sel_iso(int j, bool gen_iso)
{
  float isolation = (*bvf_["pho_ecalClusterIsoR3"])[j] + (*bvf_["pho_hcalRechitIsoR3"])[j] + (*bvf_["pho_trackIsoR3PtCut20"])[j];
  bool match = true;
  if(gen_iso && isMC_)
    {
      auto gen_index = (*bvf_["pho_genMatchedIndex"])[j];
      if(gen_index == -1) { match = false; }
      isolation = (*bvf_["mcCalIsoDR04"])[gen_index];
    }
  bool iso = (isolation <= iso_max_) && match;
  return iso;
}

#endif
