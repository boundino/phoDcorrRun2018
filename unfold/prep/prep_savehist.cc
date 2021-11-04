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
    return xjjana::cal_dphi_0pi(Dphi, jphi);
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

int prep_savehist(std::string inputname, std::string inputname_data, std::string outsubdir, std::string var,
                  Djet::param& pa)
{
  TFile* inf = TFile::Open(inputname.c_str());
  phoD::forest* f = new phoD::forest(inf, pa.ishi());
  phoD::jtree* jtr = f->jtr();
  phoD::etree* etr = f->etr();
  phoD::gtree* gtr = f->gtr();

  std::map<std::string, TH1D*> h, hrefpt;
  std::map<std::string, float> njet;
  TH1F* HDataReco = (TH1F*)(TFile::Open(inputname_data.c_str())->Get(Form("h%s_%s", var.c_str(), pa.ishi()?"sub":"pp")));
  xjjana::bins<double> vb(xjjana::gethXaxis(HDataReco));
  for(auto& t : names)
    {
      h[t+"-original"] = new TH1D(std::string(t+"-original").c_str(), Form(";%s;#frac{1}{N_{jet}} %s", Djet::vartex[var].c_str(), Djet::varytex[var].c_str()), vb.n(), vb.v().data());
      // h[t+"-original"] = new TH1D(std::string(t+"-original").c_str(), Form(";%s bin index;", Djet::vartex[var].c_str()), vb.n(), vb.v().data());
      njet[t] = 0;
    }
  for(int k=0; k<vb.n(); k++)
    {
      h["HDataReco-original"]->SetBinContent(k+1, HDataReco->GetBinContent(k+1));
      h["HDataReco-original"]->SetBinError(k+1, HDataReco->GetBinError(k+1));
    }
  TH2D* HResponse = new TH2D("HResponse", Form(";Matched %s bin index;Gen %s bin index", var.c_str(), var.c_str()), vb.n(), 0, vb.n(), vb.n(), 0, vb.n());
  std::vector<std::string> names_refpt = {"hrefpt_match_reco", "hrefpt_match_gen", 
                                          "hrefpt_match_reco_dRfilter0p10", "hrefpt_match_reco_dRfilter0p12", "hrefpt_match_reco_dRfilter0p14", "hrefpt_match_reco_dRfilter0p16", "hrefpt_match_reco_dRfilter0p18", "hrefpt_match_reco_dRfilter0p20"};
  for(auto& t : names_refpt)
    hrefpt[t] = new TH1D(t.c_str(), ";Gen jet p_{T};", 200, 0, 200);

  int nentries = f->GetEntries();
  for(int i=0; i<nentries; i++)
    {
      xjjc::progressslide(i, nentries, 100000);
      f->GetEntry(i);

      if(pa.ishi() && (etr->hiBin() < pa["centmin"]*2 || etr->hiBin() > pa["centmax"]*2)) continue;
      float weight = etr->weight();
      // float weight = 1.;
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
      for(int j=0; j<jtr->ngen(); j++)
        {
          if((*jtr)["genpt"][j] <= pa["jtptmin"] || fabs((*jtr)["geneta"][j]) >= pa["jtetamax"]) continue;
          if(jtr->val<int>("gensubid", j) != 0) continue;
          njet["HMCGen"] += weight;
          for(int k=0; k<Gphi.size(); k++)
            {
              float value = val(var, Gphi[k], Geta[k], (*jtr)["genphi"][j], (*jtr)["geneta"][j]);
              h["HMCGen-original"]->Fill(value, weight); //
            }
        }

      // match jet - [reco pt]
      for(int j=0; j<jtr->nref(); j++)
        {
          if((*jtr)["refpt"][j] < 0) continue;
          if((*jtr)["jtpt"][j] <= pa["jtptmin"] || fabs((*jtr)["jteta"][j]) >= pa["jtetamax"]) continue;
          if(jtr->val<int>("subid", j) != 0) continue;
          hrefpt["hrefpt_match_reco"]->Fill((*jtr)["refpt"][j], weight);

          float drgen = xjjana::cal_dr((*jtr)["refphi"][j], (*jtr)["refeta"][j], (*jtr)["jtphi"][j], (*jtr)["jteta"][j]);
          if(drgen < 0.10)
            hrefpt["hrefpt_match_reco_dRfilter0p10"]->Fill((*jtr)["refpt"][j], weight);
          if(drgen < 0.12)
            hrefpt["hrefpt_match_reco_dRfilter0p12"]->Fill((*jtr)["refpt"][j], weight);
          if(drgen < 0.14)
            hrefpt["hrefpt_match_reco_dRfilter0p14"]->Fill((*jtr)["refpt"][j], weight);
          if(drgen < 0.16)
            hrefpt["hrefpt_match_reco_dRfilter0p16"]->Fill((*jtr)["refpt"][j], weight);
          if(drgen < 0.18)
            hrefpt["hrefpt_match_reco_dRfilter0p18"]->Fill((*jtr)["refpt"][j], weight);
          if(drgen < 0.20)
            hrefpt["hrefpt_match_reco_dRfilter0p20"]->Fill((*jtr)["refpt"][j], weight);

          if((*jtr)["refpt"][j] < 20 || drgen > 0.14) continue; // tricky condition!

          njet["HMCMatched"] += weight;
          njet["HMCMatchedRecoptGenphi"] += weight;

          for(int k=0; k<Gphi.size(); k++)
            {
              float value_reco = val(var, Gphi[k], Geta[k], (*jtr)["jtphi"][j], (*jtr)["jteta"][j]);
              float value_gen = val(var, Gphi[k], Geta[k], (*jtr)["refphi"][j], (*jtr)["refeta"][j]);
 
              h["HMCMatched-original"]->Fill(value_reco, weight); //
              h["HMCMatchedRecoptGenphi-original"]->Fill(value_gen, weight); //
            }
        }

      // match jet - [gen pt]
      for(int j=0; j<jtr->nref(); j++)
        {
          if((*jtr)["refpt"][j] < 0) continue;
          if((*jtr)["refpt"][j] <= pa["jtptmin"] || fabs((*jtr)["refeta"][j]) >= pa["jtetamax"]) continue;
          if(jtr->val<int>("subid", j) != 0) continue;
          njet["HMCMatchedGenptRecophi"] += weight;
          njet["HMCMatchedGenptGenphi"] += weight;

          hrefpt["hrefpt_match_gen"]->Fill((*jtr)["refpt"][j], weight);

          for(int k=0; k<Gphi.size(); k++)
            {
              float value_reco = val(var, Gphi[k], Geta[k], (*jtr)["jtphi"][j], (*jtr)["jteta"][j]);
              float value_gen = val(var, Gphi[k], Geta[k], (*jtr)["refphi"][j], (*jtr)["refeta"][j]);

              h["HMCMatchedGenptRecophi-original"]->Fill(value_reco, weight); //
              h["HMCMatchedGenptGenphi-original"]->Fill(value_gen, weight); //

              // <-- Response -->
              int binx = vb.ibin(value_reco), biny = vb.ibin(value_gen);
              if(binx >= 0 && biny >= 0)
                HResponse->Fill(binx, biny, weight);
            }
        }

      // reco jet ==> No subid selection !
      // jlead = -1;
      for(int j=0; j<jtr->nref(); j++)
        {
          if((*jtr)["jtpt"][j] <= pa["jtptmin"] || fabs((*jtr)["jteta"][j]) >= pa["jtetamax"]) continue;
          // if(jtr->val<int>("subid", j) != 0) continue;
          njet["HMCReco"] += weight;
          for(int k=0; k<Gphi.size(); k++)
            {
              float value = val(var, Gphi[k], Geta[k], (*jtr)["jtphi"][j], (*jtr)["jteta"][j]);
              h["HMCReco-original"]->Fill(value, weight); //
            }
        }

    }
  xjjc::progressbar_summary(nentries);

  // normalization
  for(auto& t : names)
    {
      if(t != "HDataReco")
        {
          for(int k=0; k<vb.n(); k++)
            {
              float bin_width = vb.width(k); // dphi
              if(var=="dr") bin_width = vb.area(k); // dr
              float nj = njet[t]==0?1:njet[t];
              h[t+"-original"]->SetBinContent(k+1, h[t+"-original"]->GetBinContent(k+1)/nj/bin_width);
              h[t+"-original"]->SetBinError(k+1, h[t+"-original"]->GetBinError(k+1)/nj/bin_width);
            }
        }
    }
  for(int k=0; k<vb.n(); k++)
    {
      float bin_width_k = vb.width(k); // dphi
      if(var=="dr") bin_width_k = vb.area(k); // dr
      for(int l=0; l<vb.n(); l++)
        {
          float bin_width_l = vb.width(l); // dphi
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
  for(auto& hh : hrefpt) xjjroot::writehist(hh.second);
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
  for(auto& hh : hrefpt) delete hh.second;
  delete HResponse;

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc == 13)
    {
      Djet::param pa(atoi(argv[4]), atof(argv[5]), atof(argv[6]), atof(argv[7]), atof(argv[8]), atof(argv[9]), atof(argv[10]), atof(argv[11]), 1/*ismc*/);
      return prep_savehist(argv[1], argv[2], argv[3], argv[12], pa);
    }
  return 1;
}
