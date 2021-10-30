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
#include "mcana.h"
#include "xjjcuti.h"
#include "xjjrootuti.h"

int mcana_savehist(std::string inputname, std::string outsubdir, int isembed, Djet::param& pa)
{
  TFile* inf = TFile::Open(inputname.c_str());
  phoD::forest* f = new phoD::forest(inf, pa.ishi());
  phoD::etree* etr = f->etr();
  phoD::dtree* dtr = f->dtr();
  phoD::gtree* gtr = f->gtr();
  phoD::jtree* jtr = f->jtr();

  std::map<std::string, xjjana::bins<float>> vb;
  std::map<std::string, TH1F*> hd;
  for(auto& v : Djet::var)
    {
      vb[v] = xjjana::bins<float>(pa.ishi()?mcana_::djt_bins_aa[v]:mcana_::djt_bins_pp[v]);
      hd[v] = new TH1F(Form("h%s_gen", v.c_str()), Form(";%s;%s", Djet::vartex[v].c_str(), Djet::varytex[v].c_str()), vb[v].n(), vb[v].v().data());
    }

  float ngenjet = 0;

  int nentries = f->GetEntries();
  for(int i=0; i<nentries; i++)
    {
      xjjc::progressslide(i, nentries, 100000);
      f->GetEntry(i);

      // event selection + hlt
      if((pa.ishi() && isembed) && (etr->hiBin() < pa["centmin"]*2 || etr->hiBin() > pa["centmax"]*2)) continue;
      float weight = etr->weight();
      if(pa.ishi() && isembed) weight *= etr->Ncoll();

      // jet selection
      for(int j=0; j<jtr->ngen(); j++) 
        {
          if((*jtr)["genpt"][j] <= pa["jtptmin"] || fabs((*jtr)["geneta"][j]) >= pa["jtetamax"]) continue;
          if(!isembed && jtr->val<int>("gensubid", j) != 0) continue;
      
          ngenjet += weight;

          // D mesons
          for(int k=0; k<gtr->Gsize(); k++)
            {
              if(abs(gtr->val<int>("GpdgId", k)) != 421) continue;
              if(!isembed && gtr->val<int>("GcollisionId", k) != 0) continue;

              if(gtr->val<float>("Gpt", k) < pa["Dptmin"] || gtr->val<float>("Gpt", k) > pa["Dptmax"]) continue;
              if(fabs(gtr->val<float>("Gy", k)) > pa["Dymax"]) continue;

              // dphi calculation
              std::map<std::string, float> d;
              d["dphi"] = xjjana::cal_dphi_0pi((*gtr)["Gphi"][k],
                                              (*jtr)["genphi"][j]); // 0 ~ 1
              d["dr"] = xjjana::cal_dr((*gtr)["Gphi"][k], (*gtr)["Geta"][k],
                                       (*jtr)["genphi"][j], (*jtr)["geneta"][j]); //

              for(auto& v : Djet::var) // dphi, dr
                hd[v]->Fill(d[v], weight);
            }
        }
    }
  xjjc::progressbar_summary(nentries);

  for(auto& v : Djet::var)
    {
      std::cout<<"\e[2m -- ngenjet: "<<ngenjet<<"\e[0m"<<std::endl;
      hd[v]->SetTitle(Form("%f", ngenjet));
    }
  xjjc::prt_divider();

  std::string outputname = "rootfiles/" + outsubdir + "_" + pa.tag() + "/savehist.root";
  xjjroot::mkdir(outputname);
  TFile* outf = new TFile(outputname.c_str(), "recreate");
  for(auto& hh : hd) xjjroot::writehist(hh.second, 10);
  pa.write();
  outf->Close();

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc==13)
    {
      Djet::param pa(atoi(argv[4]), atof(argv[5]), atof(argv[6]), atof(argv[7]), atof(argv[8]), atof(argv[9]), atof(argv[10]), atof(argv[11]), atoi(argv[12]));
      return mcana_savehist(argv[1], argv[2], atoi(argv[3]), pa);
    }
  return 1;
}
