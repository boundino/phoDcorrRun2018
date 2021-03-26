#ifndef __PHOD_ETREE__
#define __PHOD_ETREE__

#include <TTree.h>

#include <string>
#include <iostream>

namespace phoD
{
  class etree
  {
  public:
    etree(TTree* nt, bool ishi, TTree* nt_hlt = 0, TTree* nt_skim = 0) : nt_(nt), nt_hlt_(nt_hlt), nt_skim_(nt_skim), ishi_(ishi)
    {  std::cout<<"\e[32;1m -- "<<__PRETTY_FUNCTION__<<"\e[0m"<<std::endl; setbranchaddress(); }
    TTree* nt() { return nt_; }
    int GetEntries() { return nt_->GetEntries(); }
    void GetEntry(int i);
    float vz() { return vz_; }
    int hiBin() { return ishi_?hiBin_:-1; }
    float Ncoll() { return ishi_?Ncoll_:(float)1.; }
    float pthat() { return pthat_; }
    float pthatweight() { return pthatweight_; }
    bool evtsel();
    bool hltsel_photon();
    bool hltsel_jet();

  private:
    TTree* nt_;
    TTree* nt_hlt_;
    TTree* nt_skim_;
    void setbranchaddress();
    float vz_;
    int hiBin_;
    float Ncoll_;
    float pthat_;
    float pthatweight_;
    int HLT_HIGEDPhoton40_v1_;
    int HLT_HIPhoton40_HoverELoose_v1_;
    int HLT_HIAK4CaloJet80_v1_;
    int HLT_HIPuAK4CaloJet80Eta5p1_v1_;
    int HLT_HIPuAK4CaloJet100Eta5p1_v1_;
    int pclusterCompatibilityFilter_;
    int pprimaryVertexFilter_;
    int phfCoincFilter2Th4_;
    int pBeamScrapingFilter_;
    int pPAprimaryVertexFilter_;
    bool ishi_;
  };
}

void phoD::etree::setbranchaddress()
{
  if(nt_->FindBranch("vz")) nt_->SetBranchAddress("vz", &vz_);
  if(nt_->FindBranch("hiBin")) nt_->SetBranchAddress("hiBin", &hiBin_);
  if(nt_->FindBranch("Ncoll")) nt_->SetBranchAddress("Ncoll", &Ncoll_);
  if(nt_->FindBranch("pthat")) nt_->SetBranchAddress("pthat", &pthat_);
  if(nt_->FindBranch("pthatweight")) nt_->SetBranchAddress("pthatweight", &pthatweight_);
  if(nt_hlt_)
    {
      if(nt_hlt_->FindBranch("HLT_HIGEDPhoton40_v1"))
        nt_hlt_->SetBranchAddress("HLT_HIGEDPhoton40_v1", &HLT_HIGEDPhoton40_v1_);
      if(nt_hlt_->FindBranch("HLT_HIPhoton40_HoverELoose_v1"))
        nt_hlt_->SetBranchAddress("HLT_HIPhoton40_HoverELoose_v1", &HLT_HIPhoton40_HoverELoose_v1_);
      if(nt_hlt_->FindBranch("HLT_HIAK4CaloJet80_v1"))
        nt_hlt_->SetBranchAddress("HLT_HIAK4CaloJet80_v1", &HLT_HIAK4CaloJet80_v1_);
      if(nt_hlt_->FindBranch("HLT_HIPuAK4CaloJet80Eta5p1_v1"))
        nt_hlt_->SetBranchAddress("HLT_HIPuAK4CaloJet80Eta5p1_v1", &HLT_HIPuAK4CaloJet80Eta5p1_v1_);
      if(nt_hlt_->FindBranch("HLT_HIPuAK4CaloJet100Eta5p1_v1"))
        nt_hlt_->SetBranchAddress("HLT_HIPuAK4CaloJet100Eta5p1_v1", &HLT_HIPuAK4CaloJet100Eta5p1_v1_);
    }
  if(nt_skim_)
    {
      if(nt_skim_->FindBranch("pclusterCompatibilityFilter"))
        nt_skim_->SetBranchAddress("pclusterCompatibilityFilter", &pclusterCompatibilityFilter_);
      if(nt_skim_->FindBranch("pprimaryVertexFilter"))
        nt_skim_->SetBranchAddress("pprimaryVertexFilter", &pprimaryVertexFilter_);
      if(nt_skim_->FindBranch("phfCoincFilter2Th4"))
        nt_skim_->SetBranchAddress("phfCoincFilter2Th4", &phfCoincFilter2Th4_);
      if(nt_skim_->FindBranch("pBeamScrapingFilter"))
        nt_skim_->SetBranchAddress("pBeamScrapingFilter", &pBeamScrapingFilter_);
      if(nt_skim_->FindBranch("pPAprimaryVertexFilter"))
        nt_skim_->SetBranchAddress("pPAprimaryVertexFilter", &pPAprimaryVertexFilter_);
    }
}

void phoD::etree::GetEntry(int i)
{
  nt_->GetEntry(i);
  if(nt_hlt_) nt_hlt_->GetEntry(i);
  if(nt_skim_) nt_skim_->GetEntry(i);
}

bool phoD::etree::evtsel()
{
  if(ishi_ && 
     vz_ > -15 && vz_ < 15 &&
     hiBin_ >= 0 && hiBin_ < 180 &&
     pclusterCompatibilityFilter_ && pprimaryVertexFilter_ && phfCoincFilter2Th4_
     ) return true; //
  if(!ishi_ && 
     vz_ > -15 && vz_ < 15 &&
     pBeamScrapingFilter_ && pPAprimaryVertexFilter_
     ) return true; //
  return false;
}

bool phoD::etree::hltsel_photon()
{
  if(ishi_ && 
     HLT_HIGEDPhoton40_v1_
     ) return true; //
  if(!ishi_ && 
     HLT_HIPhoton40_HoverELoose_v1_
     ) return true; //
  return false;
}

bool phoD::etree::hltsel_jet()
{
  if(ishi_ && 
     HLT_HIPuAK4CaloJet80Eta5p1_v1_
     ) return true; //
  if(!ishi_ && 
     HLT_HIAK4CaloJet80_v1_
     ) return true; //
  return false;
}

#endif
