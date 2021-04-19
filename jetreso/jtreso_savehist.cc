#include <TFile.h>
#include <TH1F.h>
#include <string>
#include <vector>

#include "forest.h"
#include "jtreso.h"
#include "para.h"

#include "bins.h"
#include "xjjcuti.h"

int jtreso_savehist(std::string inputname, std::string outsubdir, Djet::param& pa)
{
  TFile* inf = TFile::Open(inputname.c_str());
  phoD::forest* f = new phoD::forest(inf, pa.ishi());
  phoD::jtree* jtr = f->jtr();
  phoD::etree* etr = f->etr();

  xjjana::bins<float> vpt(jtreso::jtpts), veta(jtreso::jtetas),
    vcent(pa.ishi()?jtreso::cents_PbPb:jtreso::cents_pp);

  auto hphi = xjjc::array3d<TH1F*>(vcent.n(), veta.n(), vpt.n());
  auto heta = xjjc::array3d<TH1F*>(vcent.n(), veta.n(), vpt.n());
  for(int c=0; c<vcent.n(); c++)
    for(int e=0; e<veta.n(); e++)
      for(int p=0; p<vpt.n(); p++)
        {
          float w = pa.ishi()?jtreso::w_phi_PbPb[c]:jtreso::w_phi_pp[c];
          hphi[c][e][p] = new TH1F(Form("hphi%s_%d_%d", (pa.ishi()?Form("_%d", c):""), e, p), ";#phi^{reco} - #phi^{gen};", 40, 0-w, w);
          heta[c][e][p] = new TH1F(Form("heta%s_%d_%d", (pa.ishi()?Form("_%d", c):""), e, p), ";#eta^{reco} - #eta^{gen};", 40, 0-w, w);
        }

  int nentries = f->GetEntries();
  for(int i=0; i<nentries; i++)
    {
      if(i%100000==0 || i==nentries-1) xjjc::progressbar(i, nentries);
      f->GetEntry(i);

      if(!etr->evtsel()) continue;
      if(etr->hiBin() >= 180) continue;

      int icent = xjjc::findibin(vcent.v(), (float)(etr->hiBin()/2.));
      if(icent < 0) { std::cout<<"error: bad icent."<<std::endl; return 3; }

      for(int j=0; j<jtr->nref(); j++)
        {
          int ipt = xjjc::findibin(vpt.v(), (*jtr)["refpt"][j]);
          int ieta = xjjc::findibin(veta.v(), fabs((*jtr)["refeta"][j]));
          if(ipt < 0 || ieta < 0) continue;

          hphi[icent][ieta][ipt]->Fill((*jtr)["jtphi"][j] - (*jtr)["refphi"][j]);
          heta[icent][ieta][ipt]->Fill((*jtr)["jteta"][j] - (*jtr)["refeta"][j]);
        }
    }
  xjjc::progressbar_summary(nentries);

  std::string outputname = "rootfiles/" + outsubdir + "_" + pa.tag("ishi") + "/savehist.root";
  xjjroot::mkdir(outputname);
  TFile* outf = new TFile(outputname.c_str(), "recreate");
  for(auto& h : hphi) for(auto& hh : h) for(auto& hhh : hh) xjjroot::writehist(hhh, 15);
  for(auto& h : heta) for(auto& hh : h) for(auto& hhh : hh) xjjroot::writehist(hhh, 15);
  pa.write();
  outf->Write();
  outf->Close();

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc==4)
    {
      Djet::param pa(atoi(argv[3]), 0, 0, 0, 0, 0, 0, 0, 1);
      return jtreso_savehist(argv[1], argv[2], pa);
    }
  return 1;
}
