#ifndef __PHOD_PTREE__
#define __PHOD_PTREE__

#include <TTree.h>

#include <vector>
#include <map>
#include <string>
#include <type_traits>

namespace phoD
{
  class ptree
  {
  public:
    ptree(TTree* nt);
    TTree* nt() { return nt_; }
    float val(std::string br, int j) { return (*bvf_[br])[j]; }
    int GetEntries() { return nt_->GetEntries(); }
    void GetEntry(int i) { nt_->GetEntry(i); }
    int nPho() { return nPho_; }
    int nEle() { return nEle_; }
    bool presel(int j);
    bool sel(int j);

  private:
    TTree* nt_;
    void setbranchaddress();
    int nPho_, nEle_;
    float photon_eta_;
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
      "pho_trackIsoR3PtCut20"
    };
    std::map<std::string, std::vector<float>*> bvf_;

    const float photon_pt_min_ = 40;
    const float photon_eta_abs_ = 1.442;
    const float hovere_max_ = 0.119947;
    const float see_min_ = 0;
    const float see_max_ = 0.010392;
    const float iso_max_ = 2.099277;
  };
}

phoD::ptree::ptree(TTree* nt) : nt_(nt)
{
  std::cout<<"\e[32;1m -- "<<__PRETTY_FUNCTION__<<std::endl;

  nt_->SetBranchStatus("*", 0);
  nt_->SetBranchStatus("nEle", 1);
  nt_->SetBranchStatus("nPho", 1);
  for(auto& b : tbvf_) nt_->SetBranchStatus(b.c_str(), 1);

  for(auto& b : tbvf_) bvf_[b] = 0;
  setbranchaddress();
}

void phoD::ptree::setbranchaddress()
{
  nt_->SetBranchAddress("nPho", &nPho_);
  nt_->SetBranchAddress("nEle", &nEle_);
  for(auto& b : tbvf_) nt_->SetBranchAddress(b.c_str(), &(bvf_[b]));
}

bool phoD::ptree::presel(int j)
{
  bool basic = (*bvf_["phoEt"])[j] > photon_pt_min_ && std::abs((*bvf_["phoSCEta"])[j]) < photon_eta_abs_ && (*bvf_["phoHoverE"])[j] <= hovere_max_;
  if(basic) return true; //
  return false;
}

bool phoD::ptree::sel(int j)
{
  bool see = (*bvf_["phoSigmaIEtaIEta_2012"])[j] <= see_max_ && (*bvf_["phoSigmaIEtaIEta_2012"])[j] >= see_min_;
  bool iso = ((*bvf_["pho_ecalClusterIsoR3"])[j] + (*bvf_["pho_hcalRechitIsoR3"])[j] + (*bvf_["pho_trackIsoR3PtCut20"])[j]) <= iso_max_;
  if(see && iso) return true; //
  return false;
}

#endif
