#ifndef __READCALC_H__
#define __READCALC_H__

#include <TH1F.h>
#include <TFile.h>
#include <map>
#include <vector>
#include <string>

#include "xjjrootuti.h"
#include "pdg.h"

namespace phoD
{
  void seth(TH1F* h, float yl=0, float yh=1.4, std::string ytitle="#frac{1}{N^{#gamma}} #frac{dN^{#gammaD}}{d#phi}")
  {
    xjjroot::sethempty(h, 0, 0.1);
    if(yl < yh)
      {
        h->SetMinimum(h->GetMinimum()*yl);
        h->SetMaximum(h->GetMaximum()*yh);
      }
    h->GetXaxis()->SetNdivisions(-505);
    // if(forcemaxdigits) h->GetYaxis()->SetMaxDigits(1);
    h->GetYaxis()->SetTitle(ytitle.c_str());
  }

  class readcalch
  {
  public:
    readcalch(TFile* inf, std::string name="");
    readcalch(readcalch& r, std::string name);
    TH1F* h(std::string name) { return fh[name]; }
    TH1F* operator[](std::string name) { return fh[name]; }
    int nbin() { return fnbin; }
    void sub();
  private:
    std::map<std::string, TH1F*> fh;
    std::string fname;
    std::vector<std::string> flist = {
      "hdphi_raw_effcorr",
      "hdphi_raw_effcorr_p",
      "hdphi_raw_fitweigh",
      "hdphi_raw_fitweigh_p",
      "hdphi_raw_unweight",
      "hdphi_raw_unweight_p",
      "hdphi_bkg_effcorr",
      "hdphi_bkg_effcorr_p",
      "hdphi_bkg_fitweigh",
      "hdphi_bkg_fitweigh_p",
      "hdphi_bkg_unweight",
      "hdphi_bkg_unweight_p",
      "hdphi_sub_effcorr",
      "hdphi_sub_fitweigh",
      "hdphi_sub_unweight",
      "hdphi_sbr_effcorr",
      "hdphi_sbr_fitweigh",
      "hdphi_sbr_unweight",
      "heff_raw",
      "heff_bkg",
    };
    int fnbin;
  };

  class subcalch
  {
  public:
    subcalch(readcalch& r_PbPb, readcalch& r_emix, std::string name);
    TH1F* h(std::string name) { return fh[name]; }
    TH1F* operator[](std::string name) { return fh[name]; }
  private:
    std::map<std::string, TH1F*> fh;
    std::string fname;
    std::vector<std::string> flist = {
      "hdphi_sub_effcorr",
      "hdphi_sbr_effcorr",
      "hdphi_sub_fitweigh",
      "hdphi_sbr_fitweigh",
    };
  };

}

phoD::readcalch::readcalch(TFile* inf, std::string name) : fname(name)
{
  for(auto& t : flist)
    {
      fh[t] = (TH1F*)inf->Get(t.c_str());
      fh[t]->SetName(Form("%s%s", fname.c_str(), t.c_str()));
      seth(fh[t], 0, 1.4);
    }
  fnbin = fh["hdphi_raw_effcorr"]->GetXaxis()->GetNbins();
}

phoD::readcalch::readcalch(readcalch& r, std::string name) : fname(name)
{
  for(auto& t : flist)
    {
      fh[t] = (TH1F*)r.h(t)->Clone(t.c_str());
      fh[t]->SetName(Form("%s%s", fname.c_str(), t.c_str()));
    }
  fnbin = fh["hdphi_raw_effcorr"]->GetXaxis()->GetNbins();
}

void phoD::readcalch::sub()
{
  std::vector<std::string> hoi = {"unweight", "effcorr", "fitweigh"};
  for(auto& t : hoi)
    {
      std::string name_sub = fh["hdphi_sub_"+t]->GetName();
      delete fh["hdphi_sub_"+t];
      fh["hdphi_raw_"+t+"_p"]->Sumw2();
      fh["hdphi_sub_"+t] = (TH1F*)fh["hdphi_raw_"+t+"_p"]->Clone(name_sub.c_str());
      fh["hdphi_bkg_"+t]->Sumw2();
      fh["hdphi_sub_"+t]->Add(fh["hdphi_bkg_"+t+"_p"], -1);
      seth(fh["hdphi_sub_"+t], 0, 0);

      std::string name_sbr = fh["hdphi_sbr_"+t]->GetName();
      delete fh["hdphi_sbr_"+t];
      fh["hdphi_sbr_"+t] = (TH1F*)fh["hdphi_sub_"+t]->Clone(name_sbr.c_str());
      fh["hdphi_sbr_"+t]->Scale(1./pdg::BR_DZERO_KPI);
      seth(fh["hdphi_sbr_"+t], 0, 0);
    }
}

phoD::subcalch::subcalch(readcalch& r_PbPb, readcalch& r_emix, std::string name)
{
  fh["hdphi_sub_effcorr"] = (TH1F*)r_PbPb["hdphi_sub_effcorr"]->Clone(Form("%shemx_sub_effcorr", name.c_str()));  
  fh["hdphi_sub_effcorr"]->Add(r_emix["hdphi_sub_effcorr"], -1);
  fh["hdphi_sbr_effcorr"] = (TH1F*)r_PbPb["hdphi_sbr_effcorr"]->Clone(Form("%shemx_sbr_effcorr", name.c_str()));  
  fh["hdphi_sbr_effcorr"]->Add(r_emix["hdphi_sbr_effcorr"], -1);

  fh["hdphi_sub_fitweigh"] = (TH1F*)r_PbPb["hdphi_sub_fitweigh"]->Clone(Form("%shemx_sub_fitweigh", name.c_str()));  
  fh["hdphi_sub_fitweigh"]->Add(r_emix["hdphi_sub_fitweigh"], -1);
  fh["hdphi_sbr_fitweigh"] = (TH1F*)r_PbPb["hdphi_sbr_fitweigh"]->Clone(Form("%shemx_sbr_fitweigh", name.c_str()));  
  fh["hdphi_sbr_fitweigh"]->Add(r_emix["hdphi_sbr_fitweigh"], -1);

  for(auto& h : fh) seth(h.second, 0, 0);
}

#endif
