#ifndef __PHOD_GTREE__
#define __PHOD_GTREE__

#include <TTree.h>
#include <TDirectory.h>

#include <vector>
#include <map>
#include <string>
#include <type_traits>

#include "xjjcuti.h"

#define MAX_GEN       20000

namespace phoD
{
  class gtree
  {
  public:
    gtree(TTree* nt, bool ishi);
    gtree(TFile* outf, std::string name, bool ishi);
    TTree* nt() { return nt_; }

    // read
    template<typename T> T val(std::string br, int j);
    float* &operator[](std::string br) { return bvf_[br]; }
    bool status(std::string br) { return bvs_[br]; }
    int GetEntries() { return nt_->GetEntries(); }
    void GetEntry(int i) { nt_->GetEntry(i); }

    // tools
    int Gsize() { return Gsize_; }
    bool presel(int j);

    void ClearGsize() { if(newtree_) { Gsize_ = 0; } }
    void Fillall(gtree* nt, int j);
    void Gsizepp() { if(newtree_) Gsize_++; }
    void Fill() { if(newtree_ ){ dr_->cd(); nt_->Fill(); } }

  private:
    TTree* nt_;
    TDirectory* dr_;
    bool newtree_;
    bool ishi_;
    void setbranchaddress();
    void branch();
    int Gsize_;
    std::vector<std::string> tbvf_ = {
      "Gpt",
      "Geta",
      "Gphi",
      "Gy",
    };
    std::vector<std::string> tbvi_ = {
      "GisSignal",
      "GcollisionId",
      "GpdgId"
    };
    std::vector<std::string> tbvo_ = {
    };

    std::map<std::string, float*> bvf_;
    std::map<std::string, int*> bvi_;
    std::map<std::string, bool*> bvo_;

    std::map<std::string, bool> bvs_; //
  };
}

phoD::gtree::gtree(TTree* nt, bool ishi) : nt_(nt), ishi_(ishi)
{
  std::cout<<"\e[32;1m -- "<<__PRETTY_FUNCTION__<<"\e[0m"<<std::endl;

  for(auto& i : tbvf_) { bvf_[i] = new float[MAX_GEN]; }
  for(auto& i : tbvi_) { bvi_[i] = new int[MAX_GEN]; }
  for(auto& i : tbvo_) { bvo_[i] = new bool[MAX_GEN]; }

  newtree_ = false;
  nt_->SetBranchStatus("*", 0);
  nt_->SetBranchStatus("Gsize", 1);

  for(auto& b : tbvf_) { bvs_[b] = false; 
    if(nt_->FindBranch(b.c_str())) { nt_->SetBranchStatus(b.c_str(), 1); bvs_[b] = true; } }
  for(auto& b : tbvi_) { bvs_[b] = false; 
    if(nt_->FindBranch(b.c_str())) { nt_->SetBranchStatus(b.c_str(), 1); bvs_[b] = true; } }
  for(auto& b : tbvo_) { bvs_[b] = false; 
    if(nt_->FindBranch(b.c_str())) { nt_->SetBranchStatus(b.c_str(), 1); bvs_[b] = true; } }

  setbranchaddress();
}

phoD::gtree::gtree(TFile* outf, std::string name, bool ishi) : ishi_(ishi)
{
  std::cout<<"\e[32;1m -- "<<__PRETTY_FUNCTION__<<"\e[0m"<<std::endl;

  for(auto& i : tbvf_) { bvf_[i] = new float[MAX_GEN]; bvs_[i] = false; }
  for(auto& i : tbvi_) { bvi_[i] = new int[MAX_GEN]; bvs_[i] = false; }
  for(auto& i : tbvo_) { bvo_[i] = new bool[MAX_GEN]; bvs_[i] = false; }

  newtree_ = true;

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

void phoD::gtree::branch()
{
  nt_->Branch("Gsize", &Gsize_, "Gsize/I");
  for(auto& b : tbvf_) nt_->Branch(b.c_str(), bvf_[b], Form("%s[Gsize]/F", b.c_str()));
  for(auto& b : tbvi_) nt_->Branch(b.c_str(), bvi_[b], Form("%s[Gsize]/I", b.c_str()));
  for(auto& b : tbvo_) nt_->Branch(b.c_str(), bvo_[b], Form("%s[Gsize]/O", b.c_str()));
}

void phoD::gtree::setbranchaddress()
{
  nt_->SetBranchAddress("Gsize", &Gsize_);
  for(auto& b : tbvf_) { if(bvs_[b]) { nt_->SetBranchAddress(b.c_str(), bvf_[b]); } }
  for(auto& b : tbvi_) { if(bvs_[b]) { nt_->SetBranchAddress(b.c_str(), bvi_[b]); } }
  for(auto& b : tbvo_) { if(bvs_[b]) { nt_->SetBranchAddress(b.c_str(), bvo_[b]); } }
}

template<typename T> T phoD::gtree::val(std::string br, int j)
{
  if(std::is_same<T, float>::value) { return bvf_[br][j]; }
  if(std::is_same<T, int>::value) { return bvi_[br][j]; }
  if(std::is_same<T, bool>::value) { return bvo_[br][j]; }
  return (T)0;
}

void phoD::gtree::Fillall(gtree* nt, int j)
{
  if(!newtree_) return;
  for(auto& b : tbvf_) { if(nt->status(b)) { bvf_[b][Gsize_] = nt->val<float>(b, j); } }
  for(auto& b : tbvi_) { if(nt->status(b)) { bvi_[b][Gsize_] = nt->val<int>(b, j); } }
  for(auto& b : tbvo_) { if(nt->status(b)) { bvo_[b][Gsize_] = nt->val<bool>(b, j); } }
}

bool phoD::gtree::presel(int j)
{
  // bool p = (bvi_["GisSignal"][j]==1 || bvi_["GisSignal"][j]==2) && bvi_["GcollisionId"][j]==0;
  bool p = (abs(bvi_["GpdgId"][j]) == 421);
  return p;
}

#endif
