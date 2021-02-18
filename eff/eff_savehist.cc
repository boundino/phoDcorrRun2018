#include <TFile.h>
#include <TTree.h>
#include <iostream>
#include <iomanip>

#include "forest.h"
#include "utili.h"

#include "xjjcuti.h"
#include "xjjrootuti.h"
#include "eff.h"

int eff_savehist(std::string inputname, std::string outputname, bool ishi)
{
  TFile* inf = TFile::Open(inputname.c_str());
  phoD::forest* f = new phoD::forest(inf, ishi);
  phoD::etree* etr = f->etr();
  phoD::dtree* dtr = f->dtr();
  phoD::gtree* gtr = f->gtr();

  eff::effbins ebin(ishi);
  bool ignorehlt = true;

  std::vector<TH1F*> heff_reco(ebin.nycent(), 0), heff_gen(ebin.nycent(), 0), heff(ebin.nycent(), 0);
  for(int k=0; k<ebin.nycent(); k++)
    {
      heff_reco[k] = new TH1F(Form("heff_reco_%d", k), ";p_{T} (GeV/c);#alpha #times #epsilon", ebin.npt(), ebin.ptbins());
      heff_gen[k] = new TH1F(Form("heff_gen_%d", k), ";p_{T} (GeV/c);#alpha #times #epsilon", ebin.npt(), ebin.ptbins());
    }
  int nentries = f->GetEntries();
  for(int i=0; i<nentries; i++)
    {
      if(i%10000==0 || i==nentries-1) xjjc::progressbar(i, nentries);
      f->GetEntry(i);

      // event selection + hlt
      if(!etr->presel(ignorehlt)) continue;

      // D mesons
      for(int j=0; j<dtr->Dsize(); j++)
        {
          if(dtr->val<int>("Dgen", j) != 23333 || dtr->val<int>("DgencollisionId", j) != 0) continue;
          if(!dtr->presel(j)) continue;
          if(!dtr->tightsel(j)) continue;

          int iycent = ebin.iycent(fabs(dtr->val<float>("Dy", j)), etr->hiBin());
          if(iycent < 0) continue;
          heff_reco[iycent]->Fill(dtr->val<float>("Dpt", j));
        }

      // Gen
      for(int j=0; j<gtr->Gsize(); j++)
        {
          if(!gtr->presel(j)) continue;

          int iycent = ebin.iycent(fabs(gtr->val<float>("Gy", j)), etr->hiBin());
          if(iycent < 0) continue;
          heff_gen[iycent]->Fill(gtr->val<float>("Gpt", j));
        }
    }
  xjjc::progressbar_summary(nentries);

  for(int k=0; k<ebin.nycent(); k++)
    {
      heff_reco[k]->Sumw2();
      heff_gen[k]->Sumw2();
      heff[k] = (TH1F*)heff_reco[k]->Clone(Form("heff_%d", k));
      heff[k]->Divide(heff_gen[k]);
    }

  std::string output = "rootfiles/" + outputname + ".root";
  xjjroot::mkdir(output);
  TFile* outf = new TFile(output.c_str(), "recreate");
  for(auto& hh : heff_reco) { xjjroot::writehist(hh, 10); }
  for(auto& hh : heff_gen) { xjjroot::writehist(hh, 10); }
  for(auto& hh : heff) { xjjroot::writehist(hh, 10); }
  outf->Close();

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc==4)
    {
      return eff_savehist(argv[1], argv[2], atoi(argv[3]));
    }
  return 1;
}
