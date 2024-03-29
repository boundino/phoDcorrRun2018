#ifndef __PHOD_PTREE__
#define __PHOD_PTREE__

#include <TTree.h>
#include <TDirectory.h>

#include <vector>
#include <map>
#include <string>
#include <type_traits>

#include "para.h"

namespace phoD
{
  class ptree
  {
  public:
    ptree(TTree* nt, bool ishi);
    ptree(TFile* outf, std::string name, bool ishi, bool isMC, TTree* nt_template=0);
    TTree* nt() { return nt_; }

    // read
    template<typename T> T val(std::string br, int j);
    std::vector<float> &operator[](std::string br) { return *(bvf_[br]); }
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
    bool sel_iso(int j, bool gen_iso=false, std::string var="R3");
    bool sel_iso_gen(int j, std::string var="mcCalIsoDR04");
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
    template<typename T> bool checkbranchstatus(std::string b, TTree* nt_template=0);
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
      "pho_ecalClusterIsoR4",
      "pho_hcalRechitIsoR3",
      "pho_hcalRechitIsoR4",
      "pho_trackIsoR3PtCut20",
      "pho_trackIsoR4PtCut20",
      "mcPt",
      "mcEta",
      "mcPhi",
      "mcEt",
      "mcCalIsoDR04", // https://github.com/CmsHI/cmssw/blob/forest_CMSSW_10_3_1/HeavyIonsAnalysis/PhotonAnalysis/plugins/ggHiNtuplizer.cc#L1207
      "mcCalIsoDR03",
    };
    std::vector<std::string> tbvi_ = {
      "pho_genMatchedIndex",
      "mcPID",
      "mcStatus",
      "mcMomPID"
    };
    std::map<std::string, std::vector<float>*> bvf_;
    std::map<std::string, std::vector<int>*> bvi_;
    std::map<std::string, bool> bvs_; // if the branch is valid
  };
}

template<typename T>
bool phoD::ptree::checkbranchstatus(std::string b, TTree* nt_template)
{
  bool bvs = false;
  if(nt_template)
    {
      if(nt_template->FindBranch(b.c_str()))
        {
          bvs = true;
          std::cout<<"  \e[34m--> "<<b<<"\e[0m"<<std::endl;
        }
      else
        std::cout<<"  \e[31m(x) "<<b<<"\e[0m"<<std::endl;
    }
  else
    {
      bvs = true;
      std::cout<<"  \e[32m--> "<<b<<"\e[0m"<<std::endl;
    }
  bvs_[b] = bvs;
  return bvs;
}

// read existing trees
phoD::ptree::ptree(TTree* nt, bool ishi) : nt_(nt), ishi_(ishi)
{
  std::cout<<"\e[32;1m -- "<<__PRETTY_FUNCTION__<<"\e[0m"<<std::endl;

  newtree_ = false;
  isMC_ = nt_->FindBranch("nMC");

  nt_->SetBranchStatus("*", 0);

  // check + set [bvs]
  for(auto& b : tbvf_) { if(checkbranchstatus<float>(b, nt_)) bvf_[b] = 0; }
  for(auto& b : tbvi_) { if(checkbranchstatus<int>(b, nt_)) bvi_[b] = 0; }

  setbranchaddress();
}

// create new trees
phoD::ptree::ptree(TFile* outf, std::string name, bool ishi, bool isMC, TTree* nt_template) : isMC_(isMC), ishi_(ishi)
{
  std::cout<<"\e[32;1m -- "<<__PRETTY_FUNCTION__<<"\e[0m"<<std::endl;

  newtree_ = true;

  // check + set [bvs]
  for(auto& b : tbvf_) { if(checkbranchstatus<float>(b, nt_template)) bvf_[b] = new std::vector<float>; }
  for(auto& b : tbvi_) { if(checkbranchstatus<int>(b, nt_template)) bvi_[b] = new std::vector<int>; }

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
  for(auto& b : tbvf_) { if(bvs_[b]) bvf_[b]->clear(); }
}

void phoD::ptree::branch()
{
  nt_->Branch("nPho", &nPho_, "nPho/I");
  nt_->Branch("nEle", &nEle_, "nEle/I");
  if(isMC_) nt_->Branch("nMC", &nMC_, "nMC/I");
  for(auto& b : tbvf_) { if(bvs_[b]) { nt_->Branch(b.c_str(), bvf_[b]); } }
  for(auto& b : tbvi_) { if(bvs_[b]) { nt_->Branch(b.c_str(), bvi_[b]); } }
}

void phoD::ptree::setbranchaddress()
{
  nt_->SetBranchStatus("nPho", 1); nt_->SetBranchAddress("nPho", &nPho_);
  nt_->SetBranchStatus("nEle", 1); nt_->SetBranchAddress("nEle", &nEle_);
  if(nt_->FindBranch("nMC")) { nt_->SetBranchStatus("nMC", 1); nt_->SetBranchAddress("nMC", &nMC_); }
  for(auto& b : tbvf_) { if(bvs_[b]) { nt_->SetBranchStatus(b.c_str(), 1); nt_->SetBranchAddress(b.c_str(), &(bvf_[b])); } }
  for(auto& b : tbvi_) { if(bvs_[b]) { nt_->SetBranchStatus(b.c_str(), 1); nt_->SetBranchAddress(b.c_str(), &(bvi_[b])); } }
}

template<typename T> T phoD::ptree::val(std::string br, int j)
{
  if(std::is_same<T, float>::value) { return (*bvf_[br])[j]; }
  if(std::is_same<T, int>::value) { return (*bvi_[br])[j]; }
  // if(std::is_same<T, bool>::value) { return (*bvo_[br])[j]; }
  return (T)0;
}

void phoD::ptree::Fillall(std::string tag, ptree* nt, int j)
{
  if(!newtree_) return;
  for(auto& b : tbvf_) { if(nt->status(b) && bvs_[b] && xjjc::str_contains(b, tag)) { bvf_[b]->push_back(nt->val<float>(b, j)); } }
  for(auto& b : tbvi_) { if(nt->status(b) && bvs_[b] && xjjc::str_contains(b, tag)) { bvi_[b]->push_back(nt->val<int>(b, j)); } }
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

bool phoD::ptree::sel_iso(int j, bool gen_iso, std::string var)
{
  float isolation = (*bvf_["pho_ecalClusterIso"+var])[j] + (*bvf_["pho_hcalRechitIso"+var])[j] + (*bvf_["pho_trackIso"+var+"PtCut20"])[j];
  bool match = true;
  float iso_max_ = ishi_?iso_max_aa_:iso_max_pp_;
  if(gen_iso && isMC_)
    {
      auto gen_index = (*bvi_["pho_genMatchedIndex"])[j];
      if(gen_index == -1) { match = false; }
      std::string genvar = var=="R4"?"mcCalIsoDR04":"mcCalIsoDR03";
      isolation = (*bvf_[genvar])[gen_index];
      iso_max_ = ishi_?iso_max_aa_gen_:iso_max_pp_gen_;
    }
  bool iso = (isolation <= iso_max_) && match;
  return iso;
}

bool phoD::ptree::sel_iso_gen(int j, std::string var)
{
  float isolation = (*bvf_[var])[j];
  float iso_max_ = ishi_?iso_max_aa_gen_:iso_max_pp_gen_;
  bool iso = isolation <= iso_max_;
  return iso;
}

#endif
