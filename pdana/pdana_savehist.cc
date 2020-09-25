#include <TFile.h>
#include <TTree.h>
#include <iostream>
#include <iomanip>

#include "forest.h"
#include "param.h"
#include "bins.h"

#include "xjjcuti.h"
#include "xjjrootuti.h"
#include "dfitter.h"

int pdana_savehist(std::string inputname, std::string outsubdir, phoD::param& pa)
{
  TFile* inf = TFile::Open(inputname.c_str());
  phoD::forest* f = new phoD::forest(inf);
  phoD::etree* etr = f->etr();
  phoD::dtree* dtr = f->dtr();
  phoD::ptree* ptr = f->ptr();
  // phoD::bins<float>* vb = new phoD::bins<float>(phoD::bins_dphi);
  phoD::bins<float> vb(phoD::bins_dphi);
  std::vector<TH1F*> hmass(vb.n(), 0);
  for(int k=0; k<vb.n(); k++)
    {
      hmass[k] = new TH1F(Form("hmass_%d", k), 
                          Form("#Delta#phi/#pi: %s - %s;;", xjjc::number_remove_zero(vb[k]).c_str(), xjjc::number_remove_zero(vb[k+1]).c_str()), 
                          xjjroot::n_hist_dzero, xjjroot::min_hist_dzero, xjjroot::max_hist_dzero);
    }

  int nentries = f->GetEntries();
  int passevt = 0;
  for(int i=0; i<nentries; i++)
    {
      if(i%10000==0 || i==nentries-1) xjjc::progressbar(i, nentries);
      f->GetEntry(i);

      // event selection
      if(!etr->presel() || etr->hiBin() < pa["centmin"]*2 || etr->hiBin() > pa["centmax"]*2) continue;

      // photon selection
      int jlead = -1;
      for(int j=0; j<ptr->nPho(); j++) 
        {
          // pt, eta, 
          if(!ptr->presel(j)) continue;
          jlead = j;
          break;
        }
      if(jlead < 0) continue;
      // see, iso, 
      if(!ptr->sel(jlead)) continue;
      // miss eletron reject selection

      passevt++;

      // D mesons
      for(int j=0; j<dtr->Dsize(); j++)
        {
          if(!(dtr->presel(j))) continue;
          if(!(dtr->val<float>("Dpt", j) > pa["Dptmin"] && dtr->val<float>("Dpt", j) < pa["Dptmax"] && 
               fabs(dtr->val<float>("Dy", j)) < pa["Dymax"])) continue;
          if(dtr->val<float>("Dalpha", j) >= 0.12) continue;
          if((dtr->val<float>("DsvpvDistance", j)/dtr->val<float>("DsvpvDisErr", j)) <= 6) continue;
          if(dtr->val<float>("Dchi2cl", j) <= 0.25) continue;

          float dphi = dtr->val<float>("Dphi", j) - ptr->val("phoPhi", jlead); //
          if(dphi < -M_PI) dphi += M_PI*2;
          else if(dphi > M_PI) dphi -= M_PI*2;
          dphi = fabs(dphi)/M_PI;
          int ibin = vb.ibin(dphi);
          hmass[ibin]->Fill(dtr->val<float>("Dmass", j));
        }
    }
  xjjc::progressbar_summary(nentries);
  std::cout<<passevt<<std::endl;

  std::string outputname = "rootfiles/" + outsubdir + "_" + pa.tag() + "/savehist.root";
  xjjroot::mkdir(outputname);
  TFile* outf = new TFile(outputname.c_str(), "recreate");
  for(auto& hh : hmass) { xjjroot::printhist(hh, 9); hh->Write(); }
  pa.write();
  outf->Close();

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc==8)
    {
      phoD::param pa(atof(argv[3]), atof(argv[4]), atof(argv[5]), atof(argv[6]), atof(argv[7]));
      return pdana_savehist(argv[1], argv[2], pa);
    }
  return 1;
}
