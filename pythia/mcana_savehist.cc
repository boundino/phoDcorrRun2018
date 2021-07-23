#include <TFile.h>
#include <TTree.h>
#include <iostream>
#include <iomanip>
#include <map>
#include <vector>

#include "forest.h"
#include "param.h"
#include "bins.h"
#include "utili.h"

#include "mcana.h"

#include "xjjcuti.h"
#include "xjjrootuti.h"

int mcana_savehist(std::string inputname, std::string outsubdir, int isembed, phoD::param& pa)
{
  TFile* inf = TFile::Open(inputname.c_str());
  phoD::forest* f = new phoD::forest(inf, pa.ishi());
  phoD::etree* etr = f->etr();
  phoD::dtree* dtr = f->dtr();
  phoD::gtree* gtr = f->gtr();
  phoD::ptree* ptr = f->ptr();

  phoD::bins<float> vb(isembed?phoD::bins_dphi_aa:phoD::bins_dphi_pp);
  for(auto& tt : mcana_::types)
    mcana_::hdphi[tt] = new TH1F(Form("hdphi_%s", tt.c_str()), ";#Delta#phi^{#gammaD} / #pi;#frac{dN^{#gammaD}}{d#phi}", vb.n(), vb.v().data());
  float nphoton_mc_isoR4 = 0, nphoton_mc_isoR3 = 0, 
    nphoton_gen_isoR4 = 0, nphoton_gen_isoR3 = 0;

  int nentries = f->GetEntries();
  for(int i=0; i<nentries; i++)
    {
      xjjc::progressslide(i, nentries, 10000);
      f->GetEntry(i);

      // event selection + hlt
      if((pa.ishi() && isembed) && (etr->hiBin() < pa["centmin"]*2 || etr->hiBin() > pa["centmax"]*2)) continue;
      if(!etr->hltsel_photon() || !etr->evtsel()) continue;
      float weight = etr->pthatweight();
      if(pa.ishi() && isembed) weight *= etr->Ncoll();
      // photon selection
      int jlead = -1;
      for(int j=0; j<ptr->nPho(); j++) 
        {
          // HoverE
          if(!ptr->presel(j)) continue;
          if(ptr->val<float>("phoEt", j) <= pa["phoptmin"] || fabs(ptr->val<float>("phoSCEta", j)) >= pa["phoetamax"]) continue;
          jlead = j;
          break;
        }
      if(jlead < 0) continue;
      if(!ptr->sel_hem(jlead)) continue;
      int gen_index = ptr->val<int>("pho_genMatchedIndex", jlead);
      if(gen_index < 0) continue;
      bool fill_iso_R4 = ptr->sel_iso(jlead, false, "R4");
      bool fill_iso_R3 = ptr->sel_iso(jlead, false, "R3");

      float weight_isoR4 = fill_iso_R4?weight:0;
      float weight_isoR3 = fill_iso_R3?weight:0;

      // miss eletron reject selection

      nphoton_mc_isoR4 += weight_isoR4;
      nphoton_mc_isoR3 += weight_isoR3;

      // D mesons
      for(int j=0; j<dtr->Dsize(); j++)
        {
          // D selection applied in skim
          if(dtr->val<int>("Dgen", j) != 23333) continue;
          if(!isembed && dtr->val<int>("DgencollisionId", j) != 0) continue;

          if(dtr->val<float>("Dpt", j) < pa["Dptmin"] || dtr->val<float>("Dpt", j) > pa["Dptmax"]) continue;
          if(fabs(dtr->val<float>("Dy", j)) > pa["Dymax"]) continue;

          if(!dtr->presel(j)) continue;
          if(!dtr->tightsel(j)) continue;

          // dphi calculation
          float dphi = phoD::cal_dphi_01(dtr->val<float>("Dphi", j), 
                                         ptr->val<float>("phoPhi", jlead)); // 0 ~ 1
          mcana_::hdphi["mc_isoR4"]->Fill(dphi, weight_isoR4);
          mcana_::hdphi["mc_isoR3"]->Fill(dphi, weight_isoR3);
        }
    }
  xjjc::progressbar_summary(nentries);

  mcana_::hdphi["mc_isoR4"]->SetTitle(Form("%f", nphoton_mc_isoR4));
  mcana_::hdphi["mc_isoR3"]->SetTitle(Form("%f", nphoton_mc_isoR3));
  std::cout<<"\e[2m -- nphoton_mc_isoR4: "<<nphoton_mc_isoR4<<"\e[0m"<<std::endl;
  std::cout<<"\e[2m -- nphoton_mc_isoR3: "<<nphoton_mc_isoR3<<"\e[0m"<<std::endl;

  for(int i=0; i<nentries; i++)
    {
      xjjc::progressslide(i, nentries, 10000);
      f->GetEntry(i);

      // event selection + hlt
      if((pa.ishi() && isembed) && (etr->hiBin() < pa["centmin"]*2 || etr->hiBin() > pa["centmax"]*2)) continue;
      // if(!etr->presel(false)) continue;
      float weight = etr->pthatweight();
      if((pa.ishi() && isembed)) weight *= etr->Ncoll();

      // photon selection
      int jlead = -1;
      for(int j=0; j<ptr->nMC(); j++) 
        {
          if(ptr->val<int>("mcPID", j) != 22) continue;
          if(ptr->val<float>("mcEt", j) <= pa["phoptmin"] || fabs(ptr->val<float>("mcEta", j)) >= pa["phoetamax"]) continue;
          jlead = j;
          break;
        }
      if(jlead < 0) continue;
      bool fill_isoR4 = ptr->sel_iso_gen(jlead, "mcCalIsoDR04");
      bool fill_isoR3 = ptr->sel_iso_gen(jlead, "mcCalIsoDR03");

      float weight_isoR4 = fill_isoR4?weight:0;
      float weight_isoR3 = fill_isoR3?weight:0;

      nphoton_gen_isoR4 += weight_isoR4;
      nphoton_gen_isoR3 += weight_isoR3;

      if(!fill_isoR4 && !fill_isoR3) continue;

      // D mesons
      for(int j=0; j<gtr->Gsize(); j++)
        {
          if(abs(gtr->val<int>("GpdgId", j)) != 421) continue;
          if(!isembed && gtr->val<int>("GcollisionId", j) != 0) continue;

          if(gtr->val<float>("Gpt", j) < pa["Dptmin"] || gtr->val<float>("Gpt", j) > pa["Dptmax"]) continue;
          if(fabs(gtr->val<float>("Gy", j)) > pa["Dymax"]) continue;

          // dphi calculation
          float dphi = phoD::cal_dphi_01(gtr->val<float>("Gphi", j), 
                                         ptr->val<float>("mcPhi", jlead)); // 0 ~ 1
          mcana_::hdphi["gen_isoR4"]->Fill(dphi, weight_isoR4);
          mcana_::hdphi["gen_isoR3"]->Fill(dphi, weight_isoR3);
          if(gtr->val<int>("GisSignal", j) != 1 && gtr->val<int>("GisSignal", j) != 2) continue;
          mcana_::hdphi["gen_Kpi_isoR4"]->Fill(dphi, weight_isoR4);
          mcana_::hdphi["gen_Kpi_isoR3"]->Fill(dphi, weight_isoR3);
        }
    }
  xjjc::progressbar_summary(nentries);
  mcana_::hdphi["gen_isoR4"]->SetTitle(Form("%f", nphoton_gen_isoR4));
  mcana_::hdphi["gen_isoR3"]->SetTitle(Form("%f", nphoton_gen_isoR3));
  mcana_::hdphi["gen_Kpi_isoR4"]->SetTitle(Form("%f", nphoton_gen_isoR4));
  mcana_::hdphi["gen_Kpi_isoR3"]->SetTitle(Form("%f", nphoton_gen_isoR3));
  std::cout<<"\e[2m -- nphoton_gen_isoR4: "<<nphoton_gen_isoR4<<"\e[0m"<<std::endl;
  std::cout<<"\e[2m -- nphoton_gen_isoR3: "<<nphoton_gen_isoR3<<"\e[0m"<<std::endl;
  xjjc::prt_divider();

  std::string outputname = "rootfiles/" + outsubdir + "_" + pa.tag() + "/savehist.root";
  xjjroot::mkdir(outputname);
  TFile* outf = new TFile(outputname.c_str(), "recreate");
  for(auto& hh : mcana_::hdphi) xjjroot::writehist(hh.second, 10);
  pa.write();
  outf->Close();

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc==13)
    {
      phoD::param pa(atoi(argv[4]), atof(argv[5]), atof(argv[6]), atof(argv[7]), atof(argv[8]), atof(argv[9]), atof(argv[10]), atof(argv[11]), atoi(argv[12]));
      return mcana_savehist(argv[1], argv[2], atoi(argv[3]), pa);
    }
  return 1;
}
