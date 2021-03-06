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
}

void phoD::skimtree::Fill()
{
  fd->cd();
  ftnew->Fill();
  foutf->cd();
}

phoD::skimbranch::skimbranch(bool ishi) : fishi(ishi)
{
  fbr["Dfinder/ntDkpi"] = {
    "Dsize",
    "Dmass",
    "Dpt",
    "Deta",
    "Dphi",
    "Dy",
    "Dchi2cl",
    "Ddtheta",
    "Dalpha",
    "DsvpvDistance",
    "DsvpvDisErr",
    "DsvpvDistance_2D",
    "DsvpvDisErr_2D",
    "Dtrk1Pt",
    "Dtrk2Pt",
    "Dtrk1PtErr",
    "Dtrk2PtErr",
    "Dtrk1Eta",
    "Dtrk2Eta",
    "Dtrk1Dz1",
    "Dtrk2Dz1",
    "Dtrk1DzError1",
    "Dtrk2DzError1",
    "Dtrk1Dxy1",
    "Dtrk2Dxy1",
    "Dtrk1DxyError1",
    "Dtrk2DxyError1",
    "Dtrk1highPurity",
    "Dtrk2highPurity",
    "Dtrk1PixelHit",
    "Dtrk2PixelHit",
    "Dtrk1StripHit",
    "Dtrk2StripHit",
    "Dtrk1nStripLayer",
    "Dtrk2nStripLayer",
    "Dtrk1nPixelLayer",
    "Dtrk2nPixelLayer",
    "Dtrk1Chi2ndf",
    "Dtrk2Chi2ndf",
    "BDT",
    "Dgen",
    "DgencollisionId",
    "Dgenpt",
    "Dgeneta",
    "Dgenphi",
    "Dgeny"
  };

  fbr["HiForest/HiForestInfo"] = {"*"};

  fbr["ggHiNtuplizerGED/EventTree"] = {
    "nEle",
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
    "nPho",
    "pho*",
    "nMC",
    "mc*"
  };

  fbr["hiEvtAnalyzer/HiTree"] = {
    "*"
  };

  if(fishi)
    fbr["hltanalysis/HltTree"] = {"HLT_HIGEDPhoton*_v*", "HLT_HIPuAK4CaloJet*_v*"};
  else
    fbr["hltanalysis/HltTree"] = {"HLT_HIPhoton*_HoverELoose_v*", "HLT_HIAK4CaloJet*_v*"};
    
  if(fishi)
    fbr["hltobject/HLT_HIGEDPhoton40_v"] = {"*"};
  else
    fbr["hltobject/HLT_HIPhoton40_HoverELoose_v"] = {"*"};

  if(fishi)
    fbr["skimanalysis/HltTree"] = {
      "pprimaryVertexFilter",
      "phfCoincFilter2Th4",
      "pclusterCompatibilityFilter"
    };
  else
    fbr["skimanalysis/HltTree"] = {
      "pBeamScrapingFilter",
      "pPAprimaryVertexFilter"
    };

  fbr["Dfinder/ntGen"] = {
    "GPV*",
    "Gsize",
    "Gy",
    "Geta",
    "Gphi",
    "Gpt",
    "GpdgId",
    "GcollisionId",
    "GisSignal",
    "Gtk1pt",
    "Gtk1eta",
    "Gtk1phi",
    "Gtk2pt",
    "Gtk2eta",
    "Gtk2phi",
  };

  fbr["akPu4PFJetAnalyzer/t"] = {
    "*"
  };

}

