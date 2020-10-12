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
  phoD::forest* f = new phoD::forest(inf, pa.ishi());
  phoD::etree* etr = f->etr();
  phoD::dtree* dtr = f->dtr();
  phoD::gtree* gtr = f->gtr();
  phoD::ptree* ptr = f->ptr();

  phoD::bins<float> vb(phoD::bins_dphi_pp);
  TH1F* hdphi_mc = new TH1F("hdphi_mc", ";#Delta#phi^{#gammaD};dN^{#gammaD}/d#phi", vb.n(), vb.v().data());
  TH1F* hdphi_gen = new TH1F("hdphi_gen", ";#Delta#phi^{#gammaD};dN^{#gammaD}/d#phi", vb.n(), vb.v().data());
  TH1F* hdphi_gen_Kpi = new TH1F("hdphi_gen_Kpi", ";#Delta#phi^{#gammaD};dN^{#gammaD}/d#phi", vb.n(), vb.v().data());

  int nentries = f->GetEntries();

  for(int i=0; i<nentries; i++)
    {
      if(i%10000==0 || i==nentries-1) xjjc::progressbar(i, nentries);
      f->GetEntry(i);

      // event selection + hlt
      if(pa.ishi() && (etr->hiBin() < pa["centmin"]*2 || etr->hiBin() > pa["centmax"]*2)) continue;
      if(!etr->presel()) continue;
      float weight = etr->pthatweight() * etr->Ncoll();

      // photon selection
      int jlead = -1;
      for(int j=0; j<ptr->nPho(); j++) 
        {
          // pt, eta, HoverE
          if(!ptr->presel(j)) continue;
          if(ptr->val("phoEt", j) <= pa["phoptmin"] || fabs(ptr->val("phoSCEta", j)) >= pa["phoetamax"]) continue;
          jlead = j;
          break;
        }
      if(jlead < 0) continue;
      if(!ptr->sel_hem(jlead) || !ptr->sel_iso(jlead, true)) continue;
      // miss eletron reject selection
      bool see_raw = ptr->sel_see_raw(jlead);
      if(!see_raw) continue;

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

  for(int i=0; i<nentries; i++)
    {
      if(i%10000==0 || i==nentries-1) xjjc::progressbar(i, nentries);
      f->GetEntry(i);

      // event selection + hlt
      if(pa.ishi() && (etr->hiBin() < pa["centmin"]*2 || etr->hiBin() > pa["centmax"]*2)) continue;
      if(!etr->presel()) continue;
      float weight = etr->pthatweight() * etr->Ncoll();

      // photon selection
      int jlead = -1;
      for(int j=0; j<ptr->nMC(); j++) 
        {
          // pt, eta, HoverE
          if(ptr->val("mcPID", j) <= 21.9 && ptr->val("mcPID", j) >= 22.1) continue;
          // if(ptr->val("mcEt", j) <= pa["phoptmin"] || fabs(ptr->val("mcEta", j)) >= pa["phoetamax"]) continue;
          jlead = j;
          break;
        }
      if(jlead < 0) continue;
      if(!ptr->sel_iso_gen(jlead)) continue;

      // D mesons
      for(int j=0; j<gtr->Gsize(); j++)
        {
          if(abs(gtr->val<int>("GpdgId", j)) != 421) continue;
          if(gtr->val<int>("GcollisionId", j) != 0) continue;

          if(gtr->val<float>("Gpt", j) < pa["Dptmin"] || gtr->val<float>("Gpt", j) > pa["Dptmax"]) continue;
          if(fabs(gtr->val<float>("Gy", j)) > pa["Dymax"]) continue;

          // dphi calculation
          float dphi = phoD::cal_dphi_01(gtr->val<float>("Gphi", j), 
                                         ptr->val("mcPhi", jlead)); // 0 ~ 1
          hdphi_gen->Fill(dphi, weight);
          if(gtr->val<int>("GisSignal", j) != 1 && gtr->val<int>("GisSignal", j) != 2) continue;
          hdphi_gen_Kpi->Fill(dphi, weight);
        }
    }
  xjjc::progressbar_summary(nentries);

  std::string outputname = "rootfiles/" + outsubdir + "_" + pa.tag() + "/savehist.root";
  xjjroot::mkdir(outputname);
  TFile* outf = new TFile(outputname.c_str(), "recreate");
  xjjroot::writehist(hdphi_mc);
  xjjroot::writehist(hdphi_gen);
  xjjroot::writehist(hdphi_gen_Kpi);
  pa.write();
  outf->Close();

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc==11)
    {
      phoD::param pa(atoi(argv[3]), atof(argv[4]), atof(argv[5]), atof(argv[6]), atof(argv[7]), atof(argv[8]), atof(argv[9]), atof(argv[10]));
      return mcana_savehist(argv[1], argv[2], pa);
    }
  return 1;
}
