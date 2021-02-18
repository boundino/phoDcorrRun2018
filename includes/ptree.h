#ifndef __PHOD_PTREE__
#define __PHOD_PTREE__

#include <TTree.h>
#include <TDirectory.h>

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
    ptree(TTree* nt, bool ishi);
    ptree(TFile* outf, std::string name, bool ishi, bool isMC);
    TTree* nt() { return nt_; }

    // read
    float val(std::string br, int j) { return (*bvf_[br])[j]; }
    bool status(std::string br) { return bvs_[br]; }
    int GetEntries() { return nt_->GetEntries(); }
    void GetEntry(int i) { nt_->GetEntry(i); }

    // tools
    int nPho() { return nPho_; }
    int nEle() { return nEle_; }
    int nMC() { return nMC_; }
    bool isMC() { return isMC_; }
    bool presel(int j);
    bool sel_hem(int j);
    bool sel_see_raw(int j);
    bool sel_see_bkg(int j);
    bool sel_iso(int j, bool gen_iso);
    bool sel_iso_gen(int j);
    bool sel(int j, bool gen_iso) { return (sel_hem(j) && sel_see_raw(j) && sel_iso(j, gen_iso)); }

    // fill new file tree
    void ClearnPhoEleMC();
    void Fillall(std::string tag, ptree* nt, int j); // tag = pho, ele, mc
    void Fillone(std::string br, float val) { if(newtree_) { bvf_[br]->push_back(val); } }
    void nPhopp() { if(newtree_) nPho_++; }
    void nElepp() { if(newtree_) nEle_++; }
    void nMCpp() { if(newtree_) nMC_++; }
    void Fill() { if(newtree_ ){ dr_->cd(); nt_->Fill(); } }

  private:
    TTree* nt_;
    TDirectory* dr_;
    bool newtree_;
    bool isMC_, ishi_;
    void setbranchaddress();
    void branch();
    int nPho_, nEle_, nMC_;
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
      "mcCalIsoDR04"
    };
    std::map<std::string, std::vector<float>*> bvf_;
    std::map<std::string, bool> bvs_; // if the branch is valid
  };
}

phoD::ptree::ptree(TTree* nt, bool ishi) : nt_(nt), ishi_(ishi)
{
  std::cout<<"\e[32;1m -- "<<__PRETTY_FUNCTION__<<"\e[0m"<<std::endl;

  newtree_ = false;

  nt_->SetBranchStatus("*", 0);
  nt_->SetBranchStatus("nEle", 1);
  nt_->SetBranchStatus("nPho", 1);

  isMC_ = nt_->FindBranch("nMC");
  if(isMC_) { nt_->SetBranchStatus("nMC", 1); }

  for(auto& b : tbvf_) { bvs_[b] = false;
    if(nt_->FindBranch(b.c_str())) { nt_->SetBranchStatus(b.c_str(), 1); bvs_[b] = true; } }
  for(auto& b : tbvf_) bvf_[b] = 0;
  setbranchaddress();
}


phoD::ptree::ptree(TFile* outf, std::string name, bool ishi, bool isMC) : isMC_(isMC), ishi_(ishi)
{
  std::cout<<"\e[32;1m -- "<<__PRETTY_FUNCTION__<<"\e[0m"<<std::endl;

  newtree_ = true;

  for(auto& i : tbvf_)
    { 
      bvf_[i] = new std::vector<float>;
      bvs_[i] = false;
    }

  std::vector<std::string> p = xjjc::str_divide(name, "/");
  if(p.size() > 1)
    {
      if(!outf->cd(p[0].c_str())) dr_ = outf->mkdir(p[0].c_str());
      else { dr_ = outf->GetDirectory(p[0].c_str()); }
    }
  else { dr_ = outf; }
  dr_->cd();
  nt_ = new TTree(p.back().c_str(), "");
  branch();
  outf->cd();
}

void phoD::ptree::ClearnPhoEleMC()
{ 
  if(!newtree_) return;
  nPho_ = 0; nEle_ = 0; nMC_ = 0;
  for(auto& b : tbvf_) bvf_[b]->clear();
}

void phoD::ptree::branch()
{
  nt_->Branch("nPho", &nPho_, "nPho/I");
  nt_->Branch("nEle", &nEle_, "nEle/I");
  if(isMC_) nt_->Branch("nMC", &nMC_, "nMC/I");
  for(auto& b : tbvf_) 
    {
      if(!isMC_ && xjjc::str_contains(b, "mc")) continue;
      nt_->Branch(b.c_str(), bvf_[b]);
      // if(xjjc::str_contains(b, "pho")) nt_->Branch(b.c_str(), bvf_[b], Form("%s[nPho]/F", b.c_str()));
      // if(xjjc::str_contains(b, "ele")) nt_->Branch(b.c_str(), bvf_[b], Form("%s[nEle]/F", b.c_str()));
      // if(xjjc::str_contains(b, "mc") && isMC_) nt_->Branch(b.c_str(), bvf_[b], Form("%s[nMC]/F", b.c_str()));
    }
}

void phoD::ptree::setbranchaddress()
{
  nt_->SetBranchAddress("nPho", &nPho_);
  nt_->SetBranchAddress("nEle", &nEle_);
  if(nt_->FindBranch("nMC")) { nt_->SetBranchAddress("nMC", &nMC_); }
  for(auto& b : tbvf_) { if(bvs_[b]) { nt_->SetBranchAddress(b.c_str(), &(bvf_[b])); } }
}


void phoD::ptree::Fillall(std::string tag, ptree* nt, int j)
{
  if(!newtree_) return;
  for(auto& b : tbvf_) { if(nt->status(b) && xjjc::str_contains(b, tag)) { bvf_[b]->push_back(nt->val(b, j)); } }
}

bool phoD::ptree::presel(int j)
{
  float hovere_max_ = ishi_?hovere_max_aa_:hovere_max_pp_;
  // bool basic = (*bvf_["phoEt"])[j] > photon_pt_min_ && std::abs((*bvf_["phoSCEta"])[j]) < photon_eta_abs_ && (*bvf_["phoHoverE"])[j] <= hovere_max_;
  bool basic = (*bvf_["phoHoverE"])[j] <= hovere_max_;
  return basic;
}

bool phoD::ptree::sel_hem(int j)
{
  bool within_hem_failure_region = ((*bvf_["phoSCEta"])[j] < -1.3 && (*bvf_["phoSCPhi"])[j] < -0.87 && (*bvf_["phoSCPhi"])[j] > -1.57);
  return !(within_hem_failure_region && ishi_);
}

bool phoD::ptree::sel_see_raw(int j)
{
  float see_min_ = ishi_?see_min_aa_raw_:see_min_pp_raw_;
  float see_max_ = ishi_?see_max_aa_raw_:see_max_pp_raw_;
  bool see = (*bvf_["phoSigmaIEtaIEta_2012"])[j] <= see_max_ && (*bvf_["phoSigmaIEtaIEta_2012"])[j] >= see_min_;
  return see;
}

bool phoD::ptree::sel_see_bkg(int j)
{
  float see_min_ = ishi_?see_min_aa_bkg_:see_min_pp_bkg_;
  float see_max_ = ishi_?see_max_aa_bkg_:see_max_pp_bkg_;
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
  float iso_max_ = ishi_?iso_max_aa_:iso_max_pp_;
  bool iso = (isolation <= iso_max_) && match;
  return iso;
}

bool phoD::ptree::sel_iso_gen(int j)
{
  float isolation = (*bvf_["mcCalIsoDR04"])[j];
  float iso_max_ = ishi_?iso_max_aa_:iso_max_pp_;
  bool iso = isolation <= iso_max_;
  return iso;
}

#endif
