#ifndef __PHOD_FOREST__
#define __PHOD_FOREST__

#include <TFile.h>
#include <TTree.h>

#include "etree.h"
#include "dtree.h"
#include "ptree.h"

namespace phoD
{
  class forest
  {
  public:
    forest(TFile* inf);
    int GetEntries() { return n_; }
    void GetEntry(int i);
    etree* etr() { return etr_; }
    dtree* dtr() { return dtr_; }
    ptree* ptr() { return ptr_; }
  private:
    TFile* f_;
    etree* etr_;
    dtree* dtr_;
    ptree* ptr_;
    int n_;
  };
}

phoD::forest::forest(TFile* inf) : f_(inf)
{
  TTree* et = (TTree*)inf->Get("hiEvtAnalyzer/HiTree");
  etr_ = new etree(et);
  TTree* dt = (TTree*)inf->Get("Dfinder/ntDkpi");
  dtr_ = new dtree(dt);
  TTree* pt = (TTree*)inf->Get("ggHiNtuplizerGED/EventTree");
  ptr_ = new ptree(pt);

  n_ = etr_->GetEntries();
}

void phoD::forest::GetEntry(int i)
{
  etr_->GetEntry(i);
  dtr_->GetEntry(i);
  ptr_->GetEntry(i);
}

#endif
