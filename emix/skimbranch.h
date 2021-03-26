#include <TFile.h>
#include <TTree.h>

#include <vector>
#include <string>
#include <iostream>

#include "xjjcuti.h"

namespace phoD
{
  class skimbranch
  {
  public:
    skimbranch(bool ishi);
    std::vector<std::string> branches(std::string tr) { return fbr[tr]; }
  private:
    bool fishi;
    std::map<std::string, std::vector<std::string>> fbr;
  };
}

namespace phoD
{
  class skimtree
  {
  public:
    skimtree(std::string treename, TFile* inf, TFile* outf, std::vector<std::string> branches);
    TTree* t() { return ft; }
    void Fill();
    void GetEntry(int i) { ft->GetEntry(i); }
    int nentries() { return ft->GetEntries(); }
  private:
    TTree* ft;
    TDirectory* fd;
    TTree* ftnew;
    TFile* foutf;
    std::map<std::string, std::vector<std::string>> branches;
  };
}

phoD::skimtree::skimtree(std::string treename, TFile* inf, TFile* outf, std::vector<std::string> branches) : foutf(outf)
{
  std::cout<<"\e[4m -- [ "<<__FUNCTION__<<": "<<treename<<" ]\e[0m"<<std::endl;
  // input
  ft = (TTree*)inf->Get(treename.c_str());
  // branch skim
  ft->SetBranchStatus("*", 0);
  for(auto& b : branches) 
    { 
      if(ft->FindBranch(b.c_str()) || xjjc::str_contains(b, "*")) { std::cout<<"\e[34;1m  --> "<<b<<"\e[0m"<<std::endl; ft->SetBranchStatus(b.c_str(), 1); } 
      else { std::cout<<"\e[31;1m  --x "<<b<<"\e[0m"<<std::endl; } 
    }
  // output
  foutf->cd();
  std::vector<std::string> p = xjjc::str_divide(treename, "/");
  if(p.size() > 1)
    { fd = foutf->mkdir(p[0].c_str()); }
  else { fd = foutf; }
  fd->cd();
  ftnew = ft->CloneTree(0);
  foutf->cd();
}

void phoD::skimtree::Fill()
{
  fd->cd();
  ftnew->Fill();
  foutf->cd();
}

phoD::skimbranch::skimbranch(bool ishi) : fishi(ishi)
{
  fbr["Dfinder/ntDkpi"] = {"*"};
  fbr["Dfinder/ntGen"] = {"*"};
  fbr["HiForest/HiForestInfo"] = {"*"};
  fbr["ggHiNtuplizerGED/EventTree"] = {"*"};
  fbr["akPu4PFJetAnalyzer/t"] = {"*"};
  fbr["hiEvtAnalyzer/HiTree"] = {"*"};
  fbr["hltanalysis/HltTree"] = {"*"};
  fbr["skimanalysis/HltTree"] = {"*"};
}

