#ifndef __PHOD_FOREST__
#define __PHOD_FOREST__

#include <TFile.h>
#include <TTree.h>

#include "etree.h"
#include "dtree.h"
#include "gtree.h"
#include "ptree.h"

namespace phoD
{
  class forest
  {
  public:
    forest(TFile* inf, bool ishi);
    int GetEntries() { return n_; }
    void GetEntry(int i);
    etree* etr() { return etr_; }
    dtree* dtr() { return dtr_; }
    gtree* gtr() { return gtr_; }
    ptree* ptr() { return ptr_; }
  private:
    TFile* f_;
    etree* etr_;
    dtree* dtr_;
    gtree* gtr_;
    ptree* ptr_;
    int n_;
    bool ishi_;
  };
}

phoD::forest::forest(TFile* inf, bool ishi) : f_(inf), ishi_(ishi)
{
  TTree* et = (TTree*)inf->Get("hiEvtAnalyzer/HiTree");
  TTree* ht = (TTree*)inf->Get("hltanalysis/HltTree");
  TTree* st = (TTree*)inf->Get("skimanalysis/HltTree");
  etr_ = new etree(et, ishi_, ht, st);
  TTree* dt = (TTree*)inf->Get("Dfinder/ntDkpi");
  dtr_ = new dtree(dt, ishi_);
  TTree* gt = (TTree*)inf->Get("Dfinder/ntGen");
  gtr_ = new gtree(gt, ishi_);
  TTree* pt = (TTree*)inf->Get("ggHiNtuplizerGED/EventTree");
  ptr_ = new ptree(pt, ishi_);

  n_ = etr_->GetEntries();
}

void phoD::forest::GetEntry(int i)
{
  etr_->GetEntry(i);
  dtr_->GetEntry(i);
  gtr_->GetEntry(i);
  ptr_->GetEntry(i);
}

#endif
