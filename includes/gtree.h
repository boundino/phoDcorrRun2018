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

  private:
    TTree* nt_;
    bool ishi_;
    void setbranchaddress();
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

bool phoD::gtree::presel(int j)
{
  bool p = (bvi_["GisSignal"][j]==1 || bvi_["GisSignal"][j]==2) && bvi_["GcollisionId"][j]==0;
  return p;
}

#endif
