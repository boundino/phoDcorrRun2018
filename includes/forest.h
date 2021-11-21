#ifndef __PHOD_FOREST__
#define __PHOD_FOREST__

#include <TFile.h>
#include <TTree.h>

#include "etree.h"
#include "dtree.h"
#include "gtree.h"
#include "ptree.h"
#include "jtree.h"

namespace phoD
{
  class forest
  {
  public:
    forest(TFile* inf, bool ishi, std::string tjet=""); // tjet="ak(Pu)4PFJetAnalyzer/t"
    int GetEntries() { return n_; }
    void GetEntry(int i);
    etree* etr() { return etr_; }
    dtree* dtr() { return dtr_; }
    gtree* gtr() { return gtr_; }
    ptree* ptr() { return ptr_; }
    jtree* jtr() { return jtr_; }
  private:
    TFile* f_;
    etree* etr_;
    dtree* dtr_;
    gtree* gtr_;
    ptree* ptr_;
    jtree* jtr_;
    int n_;
    bool ishi_;
  };
}

phoD::forest::forest(TFile* inf, bool ishi, std::string tjet) : f_(inf), ishi_(ishi)
{
  TTree* et = (TTree*)inf->Get("hiEvtAnalyzer/HiTree");
  TTree* ht = (TTree*)inf->Get("hltanalysis/HltTree");
  TTree* st = (TTree*)inf->Get("skimanalysis/HltTree");
  etr_ = new etree(et, ishi_, ht, st);
  TTree* dt = (TTree*)inf->Get("Dfinder/ntDkpi");
  if(dt) dtr_ = new dtree(dt, ishi_);
  else dtr_ = 0;
  TTree* gt = (TTree*)inf->Get("Dfinder/ntGen");
  if(gt) gtr_ = new gtree(gt, ishi_);
  else gtr_ = 0;
  TTree* pt = (TTree*)inf->Get("ggHiNtuplizerGED/EventTree");
  if(pt) ptr_ = new ptree(pt, ishi_);
  else ptr_ = 0;
  // std::string jettree = tjet==""?(ishi_?"akPu4PFJetAnalyzer/t":"ak4PFJetAnalyzer/t"):tjet;
  std::string jettree = tjet;
  if(tjet=="")
    {
      if(ishi_)
        {
          if(inf->GetListOfKeys()->Contains("akFlowPuCs4PFJetAnalyzer"))
            jettree = "akFlowPuCs4PFJetAnalyzer/t";
          else if(inf->GetListOfKeys()->Contains("akCs4PFJetAnalyzer"))
            jettree = "akCs4PFJetAnalyzer/t";
          else if(inf->GetListOfKeys()->Contains("akPu4PFJetAnalyzer"))
            jettree = "akPu4PFJetAnalyzer/t";
        }
      else
        jettree = "ak4PFJetAnalyzer/t";
    }
  TTree* jt = (TTree*)inf->Get(jettree.c_str());
  if(jt) jtr_ = new jtree(jt, ishi_);
  else { jtr_ = 0; std::cout<<"error: bad jet tree name: "<<jettree<<" ."<<std::endl; }

  n_ = etr_->GetEntries();
}

void phoD::forest::GetEntry(int i)
{
  etr_->GetEntry(i);
  dtr_->GetEntry(i);
  if(gtr_) gtr_->GetEntry(i);
  if(ptr_) ptr_->GetEntry(i);
  if(jtr_) jtr_->GetEntry(i);
}

#endif
