#include <TFile.h>
#include <TH1D.h>
#include <string>
#include <vector>

#include "forest.h"
#include "utili.h"
#include "para.h"

#include "xjjanauti.h"
#include "xjjcuti.h"
#include "xjjrootuti.h"

#include "ptdif.h"

float val(std::string var, float Dphi, float Deta, float jphi, float jeta)
{
  if(var == "dphi") 
    return xjjana::cal_dphi_0pi(Dphi, jphi);
  else if(var == "dr") 
    return xjjana::cal_dr(Dphi, Deta, jphi, jeta);
  return -1;
}

int ptdif_savehist(std::string inputname, std::string outsubdir, std::string var, bool ishi, bool withNcollweight)
{
  TFile* inf = TFile::Open(inputname.c_str());
  phoD::forest* f = new phoD::forest(inf, ishi);
  phoD::jtree* jtr = f->jtr();
  phoD::etree* etr = f->etr();
  phoD::gtree* gtr = f->gtr();

  xjjana::bins<float> vb(Djet::bins[var]);
  auto hgen = xjjc::array2d<TH1D*>(vbDpt.n(), vbpt.n()), href = xjjc::array2d<TH1D*>(vbDpt.n(), vbpt.n());
  std::vector<float> njetgen(vbpt.n(), 0), njetref(vbpt.n(), 0);
  for(int p=0; p<vbDpt.n(); p++)
    for(int i=0; i<vbpt.n(); i++)
      {
        hgen[p][i] = new TH1D(Form("hgen_%d_%.0f_%.0f", p, vbpt[i], vbpt[i+1]), Form(";%s;#frac{1}{N^{jet}} %s", Djet::vartex[var].c_str(), Djet::varytex[var].c_str()), vb.n(), vb.data());
        href[p][i] = new TH1D(Form("href_%d_%.0f_%.0f", p, vbpt[i], vbpt[i+1]), Form(";%s;#frac{1}{N^{jet}} %s", Djet::vartex[var].c_str(), Djet::varytex[var].c_str()), vb.n(), vb.data());
      }
  int nentries = f->GetEntries();
  for(int i=0; i<nentries; i++)
    {
      xjjc::progressslide(i, nentries, 100000);
      f->GetEntry(i);

      if(ishi && (etr->hiBin() < 0 || etr->hiBin() > 180)) continue;
      float weight = etr->weight();
      // float weight = 1.;
      if(ishi && withNcollweight) weight *= etr->Ncoll();

      std::vector<float> Gphi, Geta; std::vector<int> Gptbin;
      for(int j=0; j<gtr->Gsize(); j++)
        {
          // D selections
          if(abs(gtr->val<int>("GpdgId", j)) != 421) continue;
          if(gtr->val<int>("GcollisionId", j) != 0) continue;

          if(fabs(gtr->val<float>("Gy", j)) > 1.2) continue;

          int iDpt = vbDpt.ibin((*gtr)["Gpt"][j]);
          if(iDpt < 0) continue;
          Gphi.push_back((*gtr)["Gphi"][j]);
          Geta.push_back((*gtr)["Geta"][j]);
          Gptbin.push_back(iDpt);
        }

      // ref jet
      for(int j=0; j<jtr->nref(); j++)
        {
          if(fabs((*jtr)["refeta"][j]) >= 1.6) continue;
          if((*jtr)["refpt"][j] < 0) continue; // gen-matched
          if((*jtr)["jtpt"][j] < 80) continue;
          int ipt = vbpt.ibin((*jtr)["refpt"][j]);
          if(ipt < 0) continue;

          njetref[ipt] += weight;
          for(int k=0; k<Gphi.size(); k++)
            {
              float value = val(var, Gphi[k], Geta[k], (*jtr)["refphi"][j], (*jtr)["refeta"][j]);
              href[Gptbin[k]][ipt]->Fill(value, weight); //
            }
        }

      // gen jet
      for(int j=0; j<jtr->ngen(); j++)
        {
          if(fabs((*jtr)["geneta"][j]) >= 1.6) continue;
          if(jtr->val<int>("gensubid", j) != 0) continue;
          int ipt = vbpt.ibin((*jtr)["genpt"][j]);
          if(ipt < 0) continue;

          njetgen[ipt] += weight;
          for(int k=0; k<Gphi.size(); k++)
            {
              float value = val(var, Gphi[k], Geta[k], (*jtr)["genphi"][j], (*jtr)["geneta"][j]);
              hgen[Gptbin[k]][ipt]->Fill(value, weight); //
            }
        }
    }
  xjjc::progressbar_summary(nentries);

  // normalization
  for(int p=0; p<vbDpt.n(); p++)
    for(int i=0; i<vbpt.n(); i++)
      {
        for(int k=0; k<vb.n(); k++)
          {
            float bin_width_k = vb.width(k); // dphi
            if(var=="dr") bin_width_k = vb.area(k); // dr
            hgen[p][i]->SetBinContent(k+1, hgen[p][i]->GetBinContent(k+1)/bin_width_k);
            hgen[p][i]->SetBinError(k+1, hgen[p][i]->GetBinError(k+1)/bin_width_k);
            href[p][i]->SetBinContent(k+1, href[p][i]->GetBinContent(k+1)/bin_width_k);
            href[p][i]->SetBinError(k+1, href[p][i]->GetBinError(k+1)/bin_width_k);
          }
        if(njetgen[i]) hgen[p][i]->Scale(1./njetgen[i]);
        if(njetref[i]) href[p][i]->Scale(1./njetref[i]);
      }

  std::string outputname = "rootfiles_checks/" + outsubdir + "_" + var + ".root";
  xjjroot::mkdir(outputname.c_str());
  TFile* outf = new TFile(outputname.c_str(), "recreate");
  for(auto& hh : hgen)
    for(auto& hhh : hh)
      xjjroot::writehist(hhh);
  for(auto& hh : href)
    for(auto& hhh : hh)
      xjjroot::writehist(hhh);
  outf->Close();

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc == 6)
    return ptdif_savehist(argv[1], argv[2], argv[3], atoi(argv[4]), atoi(argv[5]));

  return 1;
}
