#ifndef __PHOD_JTREE__
#define __PHOD_JTREE__

#include <TTree.h>
#include <TDirectory.h>

#include <vector>
#include <map>
#include <string>
#include <type_traits>

#include "xjjcuti.h"

#define MAX_XB_JET       300
// #define MAX_XB_GEN       ?

namespace phoD
{
  class jtree
  {
  public:
    jtree(TTree* nt, bool ishi);
    jtree(TFile* outf, std::string name, bool ishi, bool isMC, TTree* nt_template);
    TTree* nt() { return nt_; }

    // read
    template<typename T> T val(std::string br, int j);
    float* &operator[](std::string br) { return bvf_[br]; }
    bool status(std::string br) { return bvs_[br]; }
    int GetEntries() { return nt_->GetEntries(); }
    void GetEntry(int i) { nt_->GetEntry(i); }

    // tools
    int nref() { return nref_; }
    int ngen() { return ngen_; }
    bool isMC() { return isMC_; }
    bool presel(int j);
    bool tightsel(int j);

    // fill new file tree
    void Clearnrefngen() { if(newtree_) { nref_ = 0; ngen_ = 0; } }
    void Fillall_nref(jtree* nt, int j);
    void Fillall_ngen(jtree* nt, int j);
    void Fillone(std::string br, float val, int i=-1) { int i_ = i<0?nref_:i; if(newtree_) { bvf_[br][i_] = val; } }
    void nrefpp() { if(newtree_) nref_++; }
    void ngenpp() { if(newtree_) ngen_++; }
    void Fill() { if(newtree_ ){ dr_->cd(); nt_->Fill(); } }

  private:
    TTree* nt_;
    TDirectory* dr_;
    bool newtree_;
    bool ishi_, isMC_;
    void setbranchaddress();
    void branch();
    template<typename T> bool checkbranchstatus(std::string b, TTree* nt_template=0);
    int nref_;
    int ngen_;
    std::vector<std::string> tbvf_ = {
      "rawpt",
      "jtpt",
      "jteta",
      "jty",
      "jtphi",
      "jtm",
      "chargedMax",
      "chargedSum",
      "photonMax",
      "photonSum",
      "neutralMax",
      "neutralSum",
      "eMax",
      "eSum",
      "muMax",
      "muSum",
      "refpt",
      "refeta",
      "refphi",
      "genpt",
      "geneta",
      "geny",
      "genphi",
    };
    std::vector<std::string> tbvi_ = {
      "chargedN",
      "photonN",
      "neutralN",
      "eN",
      "muN",
      "refparton_flavor",
      "subid",
      "gensubid",
    };
    std::vector<std::string> tbvo_ = {
    };

    std::map<std::string, float*> bvf_;
    std::map<std::string, int*> bvi_;
    std::map<std::string, bool*> bvo_;

    std::map<std::string, bool> bvs_; //
  };
}

template<typename T>
bool phoD::jtree::checkbranchstatus(std::string b, TTree* nt_template)
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
      std::cout<<"  \e[34m--> "<<b<<"\e[0m"<<std::endl;
    }
  bvs_[b] = bvs;
  return bvs;
}

// read existing trees
phoD::jtree::jtree(TTree* nt, bool ishi) : nt_(nt), ishi_(ishi)
{
  std::cout<<"\e[32;1m -- "<<__PRETTY_FUNCTION__<<"\e[0m"<<std::endl;

  newtree_ = false;
  isMC_ = nt_->FindBranch("ngen");

  nt_->SetBranchStatus("*", 0);

  // check + set [bvs]
  for(auto& b : tbvf_) { if(checkbranchstatus<float>(b, nt_)) bvf_[b] = new float[MAX_XB_JET]; }
  for(auto& b : tbvi_) { if(checkbranchstatus<int>(b, nt_)) bvi_[b] = new int[MAX_XB_JET]; }
  for(auto& b : tbvo_) { if(checkbranchstatus<bool>(b, nt_)) bvo_[b] = new bool[MAX_XB_JET]; }

  setbranchaddress();
}

// create new trees
phoD::jtree::jtree(TFile* outf, std::string name, bool ishi, bool isMC, TTree* nt_template) : ishi_(ishi), isMC_(isMC)
{
  std::cout<<"\e[32;1m -- "<<__PRETTY_FUNCTION__<<"\e[0m"<<std::endl;

  newtree_ = true;
 
  // check + set [bvs]
  for(auto& b : tbvf_) { if(checkbranchstatus<float>(b, nt_template)) bvf_[b] = new float[MAX_XB_JET]; }
  for(auto& b : tbvi_) { if(checkbranchstatus<int>(b, nt_template)) bvi_[b] = new int[MAX_XB_JET]; }
  for(auto& b : tbvo_) { if(checkbranchstatus<bool>(b, nt_template)) bvo_[b] = new bool[MAX_XB_JET]; }

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

void phoD::jtree::branch()
{
  nt_->Branch("nref", &nref_, "nref/I");
  if(isMC_) nt_->Branch("ngen", &ngen_, "ngen/I");
  for(auto& b : tbvf_) { if(bvs_[b]) { 
      nt_->Branch(b.c_str(), bvf_[b], Form("%s[%s]/F", b.c_str(), xjjc::str_contains(b, "gen")?"ngen":"nref")); } }
  for(auto& b : tbvi_) { if(bvs_[b]) { 
      nt_->Branch(b.c_str(), bvi_[b], Form("%s[%s]/I", b.c_str(), xjjc::str_contains(b, "gen")?"ngen":"nref")); } }
  for(auto& b : tbvo_) { if(bvs_[b]) { 
      nt_->Branch(b.c_str(), bvo_[b], Form("%s[%s]/O", b.c_str(), xjjc::str_contains(b, "gen")?"ngen":"nref")); } }
}

void phoD::jtree::setbranchaddress()
{
  nt_->SetBranchStatus("nref", 1); nt_->SetBranchAddress("nref", &nref_);
  if(isMC_) { nt_->SetBranchStatus("ngen", 1); nt_->SetBranchAddress("ngen", &ngen_); }
  for(auto& b : tbvf_) { if(bvs_[b]) { nt_->SetBranchStatus(b.c_str(), 1); nt_->SetBranchAddress(b.c_str(), bvf_[b]); } }
  for(auto& b : tbvi_) { if(bvs_[b]) { nt_->SetBranchStatus(b.c_str(), 1); nt_->SetBranchAddress(b.c_str(), bvi_[b]); } }
  for(auto& b : tbvo_) { if(bvs_[b]) { nt_->SetBranchStatus(b.c_str(), 1); nt_->SetBranchAddress(b.c_str(), bvo_[b]); } }
}

template<typename T> T phoD::jtree::val(std::string br, int j)
{
  if(!bvs_[br]) { std::cout<<__PRETTY_FUNCTION__<<" error: bad branch ["<<br<<"]."<<std::endl; }
  if(std::is_same<T, float>::value) { return bvf_[br][j]; }
  if(std::is_same<T, int>::value) { return bvi_[br][j]; }
  if(std::is_same<T, bool>::value) { return bvo_[br][j]; }
  return (T)0;
}

void phoD::jtree::Fillall_nref(jtree* nt, int j)
{
  if(!newtree_) return;
  for(auto& b : tbvf_) { if(nt->status(b) && bvs_[b] && !xjjc::str_contains(b, "gen")) { bvf_[b][nref_] = nt->val<float>(b, j); } }
  for(auto& b : tbvi_) { if(nt->status(b) && bvs_[b] && !xjjc::str_contains(b, "gen")) { bvi_[b][nref_] = nt->val<int>(b, j); } }
  for(auto& b : tbvo_) { if(nt->status(b) && bvs_[b] && !xjjc::str_contains(b, "gen")) { bvo_[b][nref_] = nt->val<bool>(b, j); } }
}

void phoD::jtree::Fillall_ngen(jtree* nt, int j)
{
  if(!newtree_) return;
  for(auto& b : tbvf_) { if(nt->status(b) && bvs_[b] && xjjc::str_contains(b, "gen")) { bvf_[b][ngen_] = nt->val<float>(b, j); } }
  for(auto& b : tbvi_) { if(nt->status(b) && bvs_[b] && xjjc::str_contains(b, "gen")) { bvi_[b][ngen_] = nt->val<int>(b, j); } }
  for(auto& b : tbvo_) { if(nt->status(b) && bvs_[b] && xjjc::str_contains(b, "gen")) { bvo_[b][ngen_] = nt->val<bool>(b, j); } }
}

bool phoD::jtree::presel(int j)
{
  bool cut = true;
  return cut;
}

bool phoD::jtree::tightsel(int j)
{
  bool cut = true;
  return cut;
}
#endif
