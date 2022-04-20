#include <TFile.h>
#include <TTree.h>
#include <iostream>
#include <iomanip>
#include <map>
#include <vector>

#include "forest.h"
#include "para.h"
#include "bins.h"
#include "utili.h"
#include "xjjcuti.h"
#include "xjjrootuti.h"

int mcana_savehist(std::string inputname, std::string outsubdir, Djet::param& pa)
{
  bool isnoweight = xjjc::str_contains(outsubdir, "noweight"), 
    istightmatch = xjjc::str_contains(outsubdir, "tightmatch");
  std::cout<<"Option noweight: "<<(isnoweight?"\e[32mTrue":"\e[31mFalse")<<"\e[0m"<<std::endl;
  std::cout<<"Option tightmatch: "<<(istightmatch?"\e[32mTrue":"\e[31mFalse")<<"\e[0m"<<std::endl;
  xjjc::prt_divider();

  TFile* inf = TFile::Open(inputname.c_str());
  phoD::forest* f = new phoD::forest(inf, pa.ishi());
  phoD::etree* etr = f->etr();
  // phoD::dtree* dtr = f->dtr();
  phoD::gtree* gtr = f->gtr();
  phoD::jtree* jtr = f->jtr();

  // auto hnevt = new TH1F("hnevt", ";;", 1, 0, 1);
  auto hdnjet = new TH1F("hdnjet", ";;", 1, 0, 1), 
    hgnjet = new TH1F("hgnjet", ";;", 1, 0, 1);
  std::map<std::string, xjjana::bins<float>> vb;
  std::map<std::string, TH1F*> hd, hg;
  for(auto& v : Djet::var)
    {
      vb[v] = xjjana::bins<float>(pa.ishi()?Djet::bins_aa[v]:Djet::bins_pp[v]);
      hd[v] = new TH1F(Form("h%s", v.c_str()), Form(";%s;%s", Djet::vartex[v].c_str(), Djet::varytex[v].c_str()), vb[v].n(), vb[v].v().data());
      hg[v] = new TH1F(Form("h%s_gen", v.c_str()), Form(";%s;%s", Djet::vartex[v].c_str(), Djet::varytex[v].c_str()), vb[v].n(), vb[v].v().data());
    }

  int nentries = f->GetEntries();
  for(int i=0; i<nentries; i++)
    {
      xjjc::progressslide(i, nentries, 100000);
      f->GetEntry(i);

      // event selection + hlt
      if(pa.ishi() && (etr->hiBin() < pa["centmin"]*2 || etr->hiBin() > pa["centmax"]*2)) continue;
      if(etr->pthat() < 15) continue;
      float weight = etr->weight();
      if(pa.ishi()) weight *= etr->Ncoll();
      if(isnoweight) weight = 1.;

      // hnevt->Fill(0.5, weight);

      // reco jet selection
      for(int j=0; j<jtr->nref(); j++) 
        {
          if((*jtr)["jtpt"][j] <= pa["jtptmin"] || fabs((*jtr)["jteta"][j]) >= pa["jtetamax"]) continue;
          if((*jtr)["refpt"][j] < 10 && (*jtr)["refpt"][j] > 0 && istightmatch) continue;

          hdnjet->Fill(0.5, weight);
        }
      // gen jet selection
      for(int j=0; j<jtr->ngen(); j++) 
        {
          if((*jtr)["genpt"][j] <= pa["jtptmin"] || fabs((*jtr)["geneta"][j]) >= pa["jtetamax"]) continue;
          hgnjet->Fill(0.5, weight);
        }
      // D mesons
      for(int k=0; k<gtr->Gsize(); k++)
        {
          if(abs(gtr->val<int>("GpdgId", k)) != 421) continue;
          if(gtr->val<float>("Gpt", k) < pa["Dptmin"] || gtr->val<float>("Gpt", k) > pa["Dptmax"]) continue;
          if(fabs(gtr->val<float>("Gy", k)) > pa["Dymax"]) continue;
          // reco jet selection
          for(int j=0; j<jtr->nref(); j++) 
            {
              if((*jtr)["jtpt"][j] <= pa["jtptmin"] || fabs((*jtr)["jteta"][j]) >= pa["jtetamax"]) continue;
              if((*jtr)["refpt"][j] < 10 && (*jtr)["refpt"][j] > 0 && istightmatch) continue;

              // dphi calculation
              std::map<std::string, float> d;
              d["dphi"] = xjjana::cal_dphi_0pi((*gtr)["Gphi"][k],
                                               (*jtr)["jtphi"][j]); // 0 ~ 1
              d["dr"] = xjjana::cal_dr((*gtr)["Gphi"][k], (*gtr)["Geta"][k],
                                       (*jtr)["jtphi"][j], (*jtr)["jteta"][j]); //
              for(auto& v : Djet::var) // dphi, dr
                hd[v]->Fill(d[v], weight);
            }
          // gen jet selection
          for(int j=0; j<jtr->ngen(); j++) 
            {
              if((*jtr)["genpt"][j] <= pa["jtptmin"] || fabs((*jtr)["geneta"][j]) >= pa["jtetamax"]) continue;

              // dphi calculation
              std::map<std::string, float> d;
              d["dphi"] = xjjana::cal_dphi_0pi((*gtr)["Gphi"][k],
                                               (*jtr)["genphi"][j]); // 0 ~ 1
              d["dr"] = xjjana::cal_dr((*gtr)["Gphi"][k], (*gtr)["Geta"][k],
                                       (*jtr)["genphi"][j], (*jtr)["geneta"][j]); //
              for(auto& v : Djet::var) // dphi, dr
                hg[v]->Fill(d[v], weight);
            }
        }
    }
  xjjc::progressbar_summary(nentries);

  xjjc::prt_divider();

  std::string outputname = "rootfiles/" + outsubdir + "_" + pa.tag() + "/savehist.root";
  xjjroot::mkdir(outputname);
  TFile* outf = new TFile(outputname.c_str(), "recreate");
  for(auto& hh : hd)
    xjjroot::writehist(hh.second, 10);
  for(auto& hh : hg)
    xjjroot::writehist(hh.second, 10);
  xjjroot::writehist(hdnjet, 10);
  xjjroot::writehist(hgnjet, 10);
  pa.write();
  outf->Close();

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc==12)
    {
      Djet::param pa(atoi(argv[3]), atof(argv[4]), atof(argv[5]), atof(argv[6]), atof(argv[7]), atof(argv[8]), atof(argv[9]), atof(argv[10]), atoi(argv[11]));
      return mcana_savehist(argv[1], argv[2], pa);
    }
  return 1;
}
