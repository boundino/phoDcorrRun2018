#include <TFile.h>
#include <TTree.h>
#include <iostream>
#include <iomanip>

#include "forest.h"
#include "param.h"
#include "bins.h"
#include "utili.h"

#include "xjjcuti.h"
#include "xjjrootuti.h"

int mcana_savehist(std::string inputname, std::string outsubdir, phoD::param& pa)
{
  TFile* inf = TFile::Open(inputname.c_str());
  phoD::forest* f = new phoD::forest(inf);
  phoD::etree* etr = f->etr();
  phoD::dtree* dtr = f->dtr();
  phoD::ptree* ptr = f->ptr();

  phoD::bins<float> vb(phoD::bins_dphi);
  TH1F* hdphi_mc = new TH1F("hdphi_mc", ";#Delta#phi^{#gammaD};dN^{#gammaD}/d#phi", vb.n(), vb.v().data());
  TH1F* hdphi_gen = new TH1F("hdphi_gen", ";#Delta#phi^{#gammaD};dN^{#gammaD}/d#phi", vb.n(), vb.v().data());

  int nentries = f->GetEntries();
  for(int i=0; i<nentries; i++)
    {
      if(i%10000==0 || i==nentries-1) xjjc::progressbar(i, nentries);
      f->GetEntry(i);

      // event selection + hlt
      if(!etr->presel() || etr->hiBin() < pa["centmin"]*2 || etr->hiBin() > pa["centmax"]*2) continue;
      float weight = etr->pthatweight() * etr->Ncoll();

      // photon selection
      int jlead = -1;
      for(int j=0; j<ptr->nPho(); j++) 
        {
          // pt, eta, HoverE
          if(!ptr->presel(j)) continue;
          jlead = j;
          break;
        }
      if(jlead < 0) continue;
      if(!ptr->sel(jlead, pa.ishi(), true)) continue; // gen_iso = true
      // miss eletron reject selection

      // D mesons
      for(int j=0; j<dtr->Dsize(); j++)
        {
          // D selection applied in skim
          if(dtr->val<int>("Dgen", j) != 23333) continue;
          if(dtr->val<int>("DgencollisionId", j) != 0) continue;

          if(dtr->val<float>("Dpt", j) < pa["Dptmin"] || dtr->val<float>("Dpt", j) > pa["Dptmax"]) continue;
          if(fabs(dtr->val<float>("Dy", j)) > pa["Dymax"]) continue;

          // dphi calculation
          float dphi = phoD::cal_dphi_01(dtr->val<float>("Dphi", j), 
                                         ptr->val("phoPhi", jlead)); // 0 ~ 1
          hdphi_mc->Fill(dphi, weight);
        }
    }
  xjjc::progressbar_summary(nentries);

  std::string outputname = "rootfiles/" + outsubdir + "_" + pa.tag() + "/savehist.root";
  xjjroot::mkdir(outputname);
  TFile* outf = new TFile(outputname.c_str(), "recreate");
  xjjroot::writehist(hdphi_mc);
  pa.write();
  outf->Close();

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc==9)
    {
      phoD::param pa(atoi(argv[3]), atof(argv[4]), atof(argv[5]), atof(argv[6]), atof(argv[7]), atof(argv[8]));
      return mcana_savehist(argv[1], argv[2], pa);
    }
  return 1;
}
