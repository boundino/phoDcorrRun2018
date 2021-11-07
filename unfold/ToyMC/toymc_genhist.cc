#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>

#include <map>
#include <vector>
#include <iostream>

#include "xjjrootuti.h"
#include "xjjanauti.h"
#include "toymc.h"

int unfold_toymc(std::string inputname, std::string output, int ntoy = 10)
{
  TFile* inf = TFile::Open(inputname.c_str());
  std::map<std::string, TH1D*> h;
  for(auto& t : {"HDataReco", "HMCGen", "HMCMatchedGenptRecophi"})
    h[t] = (TH1D*)inf->Get(t);
  auto HResponse = (TH2D*)inf->Get("HResponse");
  h["HMCRecoMake"] = toymc::makehreco(h["HMCGen"], HResponse, "HMCRecoMake");
  std::vector<TH1D*> htoy(ntoy, 0);
  for(int i=0; i<ntoy; i++)
    htoy[i] = toymc::sample(h["HMCRecoMake"], h["HDataReco"], Form("HMCToy_%d", i));
  // htoy[i] = toymc::sample(h["HMCMatchedGenptRecophi"], h["HDataReco"], Form("HMCToy_%d", i));

  std::string outputname = "ToyMC/"+output+".root";
  xjjroot::mkdir(outputname);
  TFile* outf = new TFile(outputname.c_str(), "recreate");
  for(auto& hh : h) hh.second->Write();
  for(auto& hh : htoy) hh->Write();
  HResponse->Write();
  outf->Close();

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc==3)
    return unfold_toymc(argv[1], argv[2]);
  return 1;
}
