#ifndef __PHOD_ETREE__
#define __PHOD_ETREE__

#include <TTree.h>

#include <string>

namespace phoD
{
  class etree
  {
  public:
    etree(TTree* nt) : nt_(nt) {  std::cout<<"\e[32;1m -- "<<__PRETTY_FUNCTION__<<"\e[0m"<<std::endl; setbranchaddress(); }
    TTree* nt() { return nt_; }
    int GetEntries() { return nt_->GetEntries(); }
    void GetEntry(int i) { nt_->GetEntry(i); }
    float vz() { return vz_; }
    int hiBin() { return hiBin_; }
    bool presel();

  private:
    TTree* nt_;
    void setbranchaddress();
    float vz_;
    int hiBin_;
  };
}

void phoD::etree::setbranchaddress()
{
  nt_->SetBranchAddress("vz", &vz_);
  nt_->SetBranchAddress("hiBin", &hiBin_);
}

bool phoD::etree::presel()
{
  if(vz_ > -15 && vz_ < 15 &&
     hiBin_ >= 0 && hiBin_ <= 180) return true; //
  return false;
}

#endif
