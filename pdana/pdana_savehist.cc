#include <TFile.h>
#include <TTree.h>
#include <iostream>
#include <iomanip>

#include "forest.h"
#include "param.h"
#include "bins.h"
#include "utili.h"

#include "xjjcuti.h"
#include "xjjrootuti.h"
#include "dfitter.h"

int pdana_savehist(std::string inputname, std::string outsubdir, phoD::param& pa)
{
  TFile* inf = TFile::Open(inputname.c_str());
  phoD::forest* f = new phoD::forest(inf, pa.ishi());
  phoD::etree* etr = f->etr();
  phoD::dtree* dtr = f->dtr();
  phoD::ptree* ptr = f->ptr();

  phoD::bins<float> vb(phoD::bins_dphi);
  std::vector<TH1F*> hmass_raw(vb.n(), 0), hmass_bkg(vb.n(), 0);
  for(int k=0; k<vb.n(); k++)
    {
      hmass_raw[k] = new TH1F(Form("hmass_raw_%d", k), 
                              Form("#Delta#phi/#pi: %s - %s;m_{K#pi} (GeV/c);", xjjc::number_remove_zero(vb[k]).c_str(), xjjc::number_remove_zero(vb[k+1]).c_str()), 
                              xjjroot::n_hist_dzero, xjjroot::min_hist_dzero, xjjroot::max_hist_dzero);
      hmass_bkg[k] = new TH1F(Form("hmass_bkg_%d", k), 
                              Form("#Delta#phi/#pi: %s - %s;m_{K#pi} (GeV/c);", xjjc::number_remove_zero(vb[k]).c_str(), xjjc::number_remove_zero(vb[k+1]).c_str()), 
                              xjjroot::n_hist_dzero, xjjroot::min_hist_dzero, xjjroot::max_hist_dzero);
    }
  TH1F* hmass_incl = new TH1F("hmass_incl", ";m_{K#pi} (GeV/c);", xjjroot::n_hist_dzero, xjjroot::min_hist_dzero, xjjroot::max_hist_dzero);

  int nentries = f->GetEntries();
  int passevtraw = 0, passevtbkg = 0, passevthlt = 0;
  for(int i=0; i<nentries; i++)
    {
      if(i%10000==0 || i==nentries-1) xjjc::progressbar(i, nentries);
      f->GetEntry(i);

      // event selection + hlt
      if(pa.ishi() && (etr->hiBin() < pa["centmin"]*2 || etr->hiBin() > pa["centmax"]*2)) continue;
      if(!etr->presel()) continue;

      passevthlt++;

      // photon selection
      int jlead = -1;
      for(int j=0; j<ptr->nPho(); j++) 
        {
          // pt, eta
          if(ptr->val("phoEt", j) <= pa["phoptmin"] || fabs(ptr->val("phoSCEta", j)) >= pa["phoetamax"]) continue;
          // hovere
          if(!ptr->presel(j)) continue;
          jlead = j;
          break;
        }
      if(jlead < 0) continue;
      if(!ptr->sel_hem(jlead) || !ptr->sel_iso(jlead, false)) continue;
      bool see_raw = ptr->sel_see_raw(jlead);
      bool see_bkg = ptr->sel_see_bkg(jlead);
      // miss eletron reject selection

      if(see_raw) passevtraw++;
      if(see_bkg) passevtbkg++;

      // D mesons
      for(int j=0; j<dtr->Dsize(); j++)
        {
          // D selection applied in skim
          if(dtr->val<float>("Dpt", j) < pa["Dptmin"] || dtr->val<float>("Dpt", j) > pa["Dptmax"]) continue;
          if(fabs(dtr->val<float>("Dy", j)) > pa["Dymax"]) continue;
          if(!dtr->presel(j)) continue;
          // dphi calculation
          float dphi = phoD::cal_dphi_01(dtr->val<float>("Dphi", j), 
                                         ptr->val("phoPhi", jlead)); // 0 ~ 1
          int ibin = vb.ibin(dphi);
          if(see_raw)
            {
              hmass_incl->Fill(dtr->val<float>("Dmass", j));
              hmass_raw[ibin]->Fill(dtr->val<float>("Dmass", j));
            }
          if(see_bkg) 
            hmass_bkg[ibin]->Fill(dtr->val<float>("Dmass", j));
        }
    }
  xjjc::progressbar_summary(nentries);
  std::cout<<"Events passing HLT + event filter: \e[31m"<<passevthlt<<"\e[0m."<<std::endl;
  std::cout<<"Events with qualified photons: \e[31m"<<passevtraw<<"\e[0m."<<std::endl;
  std::cout<<"Events with decay photons: \e[31m"<<passevtbkg<<"\e[0m."<<std::endl;

  std::string outputname = "rootfiles/" + outsubdir + "_" + pa.tag() + "/savehist.root";
  xjjroot::mkdir(outputname);
  TFile* outf = new TFile(outputname.c_str(), "recreate");
  xjjroot::writehist(hmass_incl, 10);
  for(auto& hh : hmass_raw) { xjjroot::writehist(hh, 10); }
  for(auto& hh : hmass_bkg) { xjjroot::writehist(hh, 10); }
  pa.write();
  outf->Close();

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc==11)
    {
      phoD::param pa(atoi(argv[3]), atof(argv[4]), atof(argv[5]), atof(argv[6]), atof(argv[7]), atof(argv[8]), atof(argv[9]), atof(argv[10]));
      return pdana_savehist(argv[1], argv[2], pa);
    }
  return 1;
}
