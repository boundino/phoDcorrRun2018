#ifndef __READCOMPH_H__
#define __READCOMPH_H__

#include <TH1F.h>
#include <TFile.h>
#include <map>
#include <vector>
#include <string>

#include "xjjrootuti.h"

namespace Djet
{
  class readcomph
  {
  public:
    readcomph(TFile* inf, std::vector<std::string>& var, std::string name="");
    readcomph(readcomph& r, std::string name);
    TH1F* h(std::string var, std::string name) { return fh[var][name]; }
    void setgr(std::string var, std::string name, TGraphAsymmErrors* g) { fgr[var][name] = g; }
    TGraphAsymmErrors* gr(std::string var, std::string name) { return fgr[var][name]; }
    const std::vector<std::string>& var() { return fvar; }
    const std::vector<std::string>& list() { return flist; }
    bool status(std::string name) { return fstatus[name]; }
  private:
    std::vector<std::string> fvar;
    std::map<std::string, std::map<std::string, TH1F*>> fh;
    std::map<std::string, std::map<std::string, TGraphAsymmErrors*>> fgr;
    std::map<std::string, bool> fstatus;
    std::string fname;
    void seth(TH1F* h);
    std::vector<std::string> flist = {
      "pp",
      "PbPb",
      "MC_pp",
      "MC_PbPb",
      "emix",
      "sub",
    };
  };
}

Djet::readcomph::readcomph(TFile* inf, std::vector<std::string>& var, std::string name) : fname(name), fvar(var)
{
  for(auto& v : fvar)
    {
      for(auto& t : flist)
        {
          fgr[v][t] = 0;
          fstatus[t] = false;
          fh[v][t] = xjjroot::gethist<TH1F>(inf, "h"+v+"_"+t);
          if(fh[v][t]) fstatus[t] = true;
          else continue;

          fh[v][t]->SetName(Form("%s%s", fname.c_str(), fh[v][t]->GetName()));
          seth(fh[v][t]);
        }
    }
}

Djet::readcomph::readcomph(readcomph& r, std::string name) : fname(name)
{
  fvar = r.var();
  for(auto& v : fvar)
    {
      for(auto& t : flist)
        {
          if(!r.status(t)) { fh[v][t] = 0; continue; }
          fh[v][t] = (TH1F*)r.h(v, t)->Clone(Form("%sh%s_%s", fname.c_str(), v.c_str(), t.c_str()));
          seth(fh[v][t]);
          fgr[v][t] = 0;
          fstatus[t] = true;
        }
    }
}

void Djet::readcomph::seth(TH1F* h)
{
  xjjroot::sethempty(h, 0, 0.1);
  h->GetXaxis()->SetNdivisions(-505);
}

#endif
