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

  std::map<std::string, xjjana::bins<float>> vb;
  const int NTYPE = 6;
  std::map<std::string, TH1F*> hd[NTYPE], hg[NTYPE];
  for(auto& v : Djet::var)
    {
      vb[v] = xjjana::bins<float>(pa.ishi()?Djet::bins_aa[v]:Djet::bins_pp[v]);
      for(int k=0; k<NTYPE; k++)
        {
          hd[k][v] = new TH1F(Form("h%s_%d", v.c_str(), k), Form(";%s;%s", Djet::vartex[v].c_str(), Djet::varytex[v].c_str()), vb[v].n(), vb[v].v().data());
          hg[k][v] = new TH1F(Form("h%s_gen_%d", v.c_str(), k), Form(";%s;%s", Djet::vartex[v].c_str(), Djet::varytex[v].c_str()), vb[v].n(), vb[v].v().data());
        }
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
              float drgen = xjjana::cal_dr((*jtr)["refphi"][j], (*jtr)["refeta"][j], (*jtr)["jtphi"][j], (*jtr)["jteta"][j]);
              int kindex = 2*(gtr->val<int>("GcollisionId", k) != 0) + (int)(jtr->val<int>("subid", j) != 0);
              if(kindex == 3 
                 && gtr->val<int>("GcollisionId", k) == jtr->val<int>("subid", j) 
                 && gtr->val<int>("GcollisionId", k) > 0) kindex = 4;
              // dphi calculation
              std::map<std::string, float> d;
              d["dphi"] = xjjana::cal_dphi_0pi((*gtr)["Gphi"][k],
                                               (*jtr)["jtphi"][j]); // 0 ~ 1
              d["dr"] = xjjana::cal_dr((*gtr)["Gphi"][k], (*gtr)["Geta"][k],
                                       (*jtr)["jtphi"][j], (*jtr)["jteta"][j]); //
              for(auto& v : Djet::var) // dphi, dr
                {
                  hd[kindex][v]->Fill(d[v], weight);
                  hd[5][v]->Fill(d[v], weight);
                }
            }
          // gen jet selection
          for(int j=0; j<jtr->ngen(); j++) 
            {
              if((*jtr)["genpt"][j] <= pa["jtptmin"] || fabs((*jtr)["geneta"][j]) >= pa["jtetamax"]) continue;
              int kindex = 2*(gtr->val<int>("GcollisionId", k) != 0) + (int)(jtr->val<int>("gensubid", j) != 0);
              if(kindex == 3 
                 && gtr->val<int>("GcollisionId", k) == jtr->val<int>("gensubid", j) 
                 && gtr->val<int>("GcollisionId", k) > 0) kindex = 4;
              // dphi calculation
              std::map<std::string, float> d;
              d["dphi"] = xjjana::cal_dphi_0pi((*gtr)["Gphi"][k],
                                               (*jtr)["genphi"][j]); // 0 ~ 1
              d["dr"] = xjjana::cal_dr((*gtr)["Gphi"][k], (*gtr)["Geta"][k],
                                       (*jtr)["genphi"][j], (*jtr)["geneta"][j]); //
              for(auto& v : Djet::var) // dphi, dr
                {
                  hg[kindex][v]->Fill(d[v], weight);
                  hg[5][v]->Fill(d[v], weight);
                }
            }
        }
    }
  xjjc::progressbar_summary(nentries);

  xjjc::prt_divider();

  std::string outputname = "rootfiles/" + outsubdir + "_" + pa.tag() + "/savehist.root";
  xjjroot::mkdir(outputname);
  TFile* outf = new TFile(outputname.c_str(), "recreate");
  for(auto& hm : hd)
    for(auto& hh : hm)
      xjjroot::writehist(hh.second, 10);
  for(auto& hm : hg)
    for(auto& hh : hm)
      xjjroot::writehist(hh.second, 10);
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
