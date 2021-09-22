#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>
#include <string>
#include <vector>

#include "forest.h"
#include "utili.h"
#include "para.h"
#include "bins.h"

#include "xjjanauti.h"
#include "xjjcuti.h"
#include "xjjrootuti.h"

float val(std::string var, float Dphi, float Deta, float jphi, float jeta)
{
  if(var == "dphi") 
    return xjjana::cal_dphi_01(Dphi, jphi);
  else if(var == "dr") 
    return xjjana::cal_dr(Dphi, Deta, jphi, jeta);
  return -1;
}

std::vector<std::string> names = {
  "HMCGen", "HMCReco", "HDataReco",
  "HMCMatched", 
  "HMCMatchedRecoptGenphi", "HMCMatchedGenptRecophi", "HMCMatchedGenptGenphi",
  "HMCMatchedGenBin", "HMCRecoGenBin", 
  "HGenPrimaryBinMin", "HGenPrimaryBinMax", 
  "HGenBinningBinMin", "HGenBinningBinMax",
  "HRecoPrimaryBinMin", "HRecoPrimaryBinMax",
  "HRecoBinningBinMin", "HRecoBinningBinMax",
  "HMatchedPrimaryBinMin", "HMatchedPrimaryBinMax",
  "HMatchedBinningBinMin", "HMatchedBinningBinMax"
};

int prep_savehist(std::string inputname, std::string inputname_data, std::string outsubdir, 
                  std::string var, Djet::param& pa)
{
  TFile* inf = TFile::Open(inputname.c_str());
  phoD::forest* f = new phoD::forest(inf, pa.ishi());
  phoD::jtree* jtr = f->jtr();
  phoD::etree* etr = f->etr();
  phoD::gtree* gtr = f->gtr();

  std::map<std::string, TH1D*> h;
  std::map<std::string, float> njet;
  auto dataname = xjjc::str_divide(inputname_data, ":");
  TH1F* HDataReco = (TH1F*)(TFile::Open(dataname[0].c_str())->Get(dataname[1].c_str()));
  xjjana::bins<double> vb(xjjana::gethXaxis(HDataReco));
  for(auto& t : names)
    {
      h[t+"-original"] = new TH1D(std::string(t+"-original").c_str(), Form(";%s bin index;", var.c_str()), vb.n(), vb.v().data());
      njet[t] = 0;
    }
  for(int k=0; k<vb.n(); k++)
    {
      h["HDataReco-original"]->SetBinContent(k+1, HDataReco->GetBinContent(k+1));
      h["HDataReco-original"]->SetBinError(k+1, HDataReco->GetBinError(k+1));
    }
  TH2D* HResponse = new TH2D("HResponse", ";Matched;Gen", vb.n(), 0, vb.n(), vb.n(), 0, vb.n());

  int nentries = f->GetEntries();
  for(int i=0; i<nentries; i++)
    {
      xjjc::progressslide(i, nentries, 100000);
      f->GetEntry(i);

      if(pa.ishi() && (etr->hiBin() < pa["centmin"]*2 || etr->hiBin() > pa["centmax"]*2)) continue;
      float weight = 1.;
      if(pa.ishi()) weight *= etr->Ncoll();

      std::vector<float> Gphi, Geta;
      for(int j=0; j<gtr->Gsize(); j++)
        {
          // D selections
          if(abs(gtr->val<int>("GpdgId", j)) != 421) continue;
          if(gtr->val<int>("GcollisionId", j) != 0) continue;

          if(gtr->val<float>("Gpt", j) < pa["Dptmin"] || gtr->val<float>("Gpt", j) > pa["Dptmax"]) continue;
          if(fabs(gtr->val<float>("Gy", j)) > pa["Dymax"]) continue;

          Gphi.push_back((*gtr)["Gphi"][j]);
          Geta.push_back((*gtr)["Geta"][j]);
        }

      // gen jet
      int jlead = -1;
      for(int j=0; j<jtr->ngen(); j++)
        {
          if((*jtr)["genpt"][j] <= pa["jtptmin"] || fabs((*jtr)["geneta"][j]) >= pa["jtetamax"]) continue;
          if(jtr->val<int>("gensubid", j) != 0) continue;
          jlead = j;
          break;
        }
      if(jlead >= 0) 
        {
          njet["HMCGen"] += weight;
          for(int k=0; k<Gphi.size(); k++)
            {
              float value = val(var, Gphi[k], Geta[k], (*jtr)["genphi"][jlead], (*jtr)["geneta"][jlead]);
              h["HMCGen-original"]->Fill(value, weight); //
            }
        }

      // reco jet ==> No subid selection !
      jlead = -1;
      for(int j=0; j<jtr->nref(); j++)
        {
          if((*jtr)["jtpt"][j] <= pa["jtptmin"] || fabs((*jtr)["jteta"][j]) >= pa["jtetamax"]) continue;
          // if(jtr->val<int>("subid", j) != 0) continue;
          jlead = j;
          break;
        }
      if(jlead >= 0) 
        {
          njet["HMCReco"] += weight;
          for(int k=0; k<Gphi.size(); k++)
            {
              float value = val(var, Gphi[k], Geta[k], (*jtr)["jtphi"][jlead], (*jtr)["jteta"][jlead]);
              h["HMCReco-original"]->Fill(value, weight); //
            }
        }

      // match jet - [reco pt]
      jlead = -1;
      for(int j=0; j<jtr->nref(); j++)
        {
          if((*jtr)["refpt"][j] < 0) continue;
          if((*jtr)["jtpt"][j] <= pa["jtptmin"] || fabs((*jtr)["jteta"][j]) >= pa["jtetamax"]) continue;
          if(jtr->val<int>("subid", j) != 0) continue;
          jlead = j;
          break;
        }
      if(jlead >= 0) 
        {
          njet["HMCMatched"] += weight;
          njet["HMCMatchedRecoptGenphi"] += weight;
          for(int k=0; k<Gphi.size(); k++)
            {
              float value_reco = val(var, Gphi[k], Geta[k], (*jtr)["jtphi"][jlead], (*jtr)["jteta"][jlead]);
              float value_gen = val(var, Gphi[k], Geta[k], (*jtr)["refphi"][jlead], (*jtr)["refeta"][jlead]);
              int binx = vb.ibin(value_reco), biny = vb.ibin(value_gen);
              if(binx >= 0 && biny >= 0)
                HResponse->Fill(binx, biny, weight);

              h["HMCMatched-original"]->Fill(value_reco, weight); //
              h["HMCMatchedRecoptGenphi-original"]->Fill(value_gen, weight); //
            }
        }

      // match jet - [gen pt]
      jlead = -1;
      for(int j=0; j<jtr->nref(); j++)
        {
          if((*jtr)["refpt"][j] <= pa["jtptmin"] || fabs((*jtr)["refeta"][j]) >= pa["jtetamax"]) continue;
          if(jtr->val<int>("subid", j) != 0) continue;
          jlead = j;
          break;
        }
      if(jlead >= 0) 
        {
          njet["HMCMatchedGenptRecophi"] += weight;
          njet["HMCMatchedGenptGenphi"] += weight;
          for(int k=0; k<Gphi.size(); k++)
            {
              float value_reco = val(var, Gphi[k], Geta[k], (*jtr)["jtphi"][jlead], (*jtr)["jteta"][jlead]);
              float value_gen = val(var, Gphi[k], Geta[k], (*jtr)["refphi"][jlead], (*jtr)["refeta"][jlead]);

              h["HMCMatchedGenptRecophi-original"]->Fill(value_reco, weight); //
              h["HMCMatchedGenptGenphi-original"]->Fill(value_gen, weight); //
            }
        }
    }
  xjjc::progressbar_summary(nentries);

  // normalization
  for(auto& t : names)
    {
      h[t] = (TH1D*)xjjana::changebin(h[t+"-original"], 0, vb.n(), t);
      if(t == "HDataReco") continue;
      for(int k=0; k<vb.n(); k++)
        {
          float bin_width = vb.width(k) * M_PI; // dphi
          if(var=="dr") bin_width = vb.area(k); // dr
          float nj = njet[t]==0?1:njet[t];
          h[t+"-original"]->SetBinContent(k+1, h[t+"-original"]->GetBinContent(k+1)/nj/bin_width);
          h[t+"-original"]->SetBinError(k+1, h[t+"-original"]->GetBinError(k+1)/nj/bin_width);
        }
    }
  for(int k=0; k<vb.n(); k++)
    {
      float bin_width_k = vb.width(k) * M_PI; // dphi
      if(var=="dr") bin_width_k = vb.area(k); // dr
      for(int l=0; l<vb.n(); l++)
        {
          float bin_width_l = vb.width(l) * M_PI; // dphi
          if(var=="dr") bin_width_l = vb.area(l); // dr

          HResponse->SetBinContent(k+1, l+1, HResponse->GetBinContent(k+1, l+1)/bin_width_k/bin_width_l);
          HResponse->SetBinError(k+1, l+1, HResponse->GetBinError(k+1, l+1)/bin_width_k/bin_width_l);
        }
    }

  std::string outputname = "rootfiles/" + outsubdir + "_" + var + ".root";
  xjjroot::mkdir(outputname.c_str());
  TFile* outf = new TFile(outputname.c_str(), "recreate");
  xjjroot::writehist(HResponse);
  for(auto& hh : h) xjjroot::writehist(hh.second);
  std::vector<std::string> nameds = {
    "DataEventCount", "MCEventCount",
    "DataBaselineEventCount", "MCBaselineEventCount", 
    "DataAllEventCount", "MCAllEventCount"
  };
  for(auto& t : nameds)
    {
      TNamed* tn = new TNamed(t.c_str(), "");
      tn->Write();
    }
  pa.write();
  outf->Close();

  for(auto& hh : h) delete hh.second;
  delete HResponse;

  return 0;
}

int prep_addinfo(std::string inputname, std::string outputdir)
{
  std::cout<<inputname<<std::endl;
  TFile* inf = TFile::Open(inputname.c_str());
  Djet::param* pa = new Djet::param(inf);
  std::map<std::string, TH1D*> h;
  for(auto& t : names)
    {
      h[t+"-original"] = (TH1D*)inf->Get(std::string(t+"-original").c_str());
      int n = xjjana::gethXn(h[t+"-original"]);
      h[t] = (TH1D*)xjjana::changebin(h[t+"-original"], 0, n, t);
    }

  std::string outputname = outputdir + "/" + xjjc::str_divide_lastel(inputname, "/");
  xjjroot::mkdir(outputname);
  TFile* outf = new TFile(outputname.c_str(), "recreate");
  ((TH2D*)inf->Get("HResponse"))->Write();
  for(auto& t : names)
    {
      h[t+"-original"]->Write();
      h[t]->Write();
    }
  pa->write();
  outf->Close();
  return 0;
}

int main(int argc, char* argv[])
{
  if(argc == 12)
    {
      Djet::param pa(atoi(argv[4]), atof(argv[5]), atof(argv[6]), atof(argv[7]), atof(argv[8]), atof(argv[9]), atof(argv[10]), atof(argv[11]), 1/*ismc*/);
      prep_savehist(argv[1], Form("%s:hdphi_%s", argv[2], atoi(argv[4])?"sub":"pp"), argv[3],
                    "dphi", pa);
      prep_savehist(argv[1], Form("%s:hdr_%s", argv[2], atoi(argv[4])?"sub":"pp"), argv[3],
                    "dr", pa);
      return 0;
    }

  if(argc == 3)
    return prep_addinfo(argv[1], argv[2]);

  return 1;
}
