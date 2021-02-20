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
#include "eff.h"

int pdana_savehist(std::string inputname, std::string outsubdir, phoD::param& pa)
{
  TFile* inf = TFile::Open(inputname.c_str());
  phoD::forest* f = new phoD::forest(inf, pa.ishi());
  phoD::etree* etr = f->etr();
  phoD::dtree* dtr = f->dtr();
  phoD::ptree* ptr = f->ptr();
  bool ignorehlt = xjjc::str_contains(inputname, "Bias");

  eff::effbins ebin(pa.ishi()?"effrootfiles/eff_PbPb_prompt.root":"effrootfiles/eff_pp_prompt.root", pa.ishi());

  phoD::bins<float> vb(pa.ishi()?phoD::bins_dphi_aa:phoD::bins_dphi_pp);
  std::vector<TH1F*> hmass_raw(vb.n(), 0), hmass_bkg(vb.n(), 0), hmass_raw_unweight(vb.n(), 0), hmass_bkg_unweight(vb.n(), 0);
  for(int k=0; k<vb.n(); k++)
    {
      hmass_raw[k] = new TH1F(Form("hmass_raw_%d", k), 
                              Form("#Delta#phi/#pi: %s - %s;m_{K#pi} (GeV/c);", xjjc::number_remove_zero(vb[k]).c_str(), xjjc::number_remove_zero(vb[k+1]).c_str()), 
                              xjjroot::n_hist_dzero, xjjroot::min_hist_dzero, xjjroot::max_hist_dzero);
      hmass_bkg[k] = new TH1F(Form("hmass_bkg_%d", k), 
                              Form("#Delta#phi/#pi: %s - %s;m_{K#pi} (GeV/c);", xjjc::number_remove_zero(vb[k]).c_str(), xjjc::number_remove_zero(vb[k+1]).c_str()), 
                              xjjroot::n_hist_dzero, xjjroot::min_hist_dzero, xjjroot::max_hist_dzero);
      hmass_raw_unweight[k] = new TH1F(Form("hmass_raw_unweight_%d", k), 
                              Form("#Delta#phi/#pi: %s - %s;m_{K#pi} (GeV/c);", xjjc::number_remove_zero(vb[k]).c_str(), xjjc::number_remove_zero(vb[k+1]).c_str()), 
                              xjjroot::n_hist_dzero, xjjroot::min_hist_dzero, xjjroot::max_hist_dzero);
      hmass_bkg_unweight[k] = new TH1F(Form("hmass_bkg_unweight_%d", k), 
                              Form("#Delta#phi/#pi: %s - %s;m_{K#pi} (GeV/c);", xjjc::number_remove_zero(vb[k]).c_str(), xjjc::number_remove_zero(vb[k+1]).c_str()), 
                              xjjroot::n_hist_dzero, xjjroot::min_hist_dzero, xjjroot::max_hist_dzero);
    }
  TH1F* hmass_incl_raw = new TH1F("hmass_incl_raw", ";m_{K#pi} (GeV/c);", xjjroot::n_hist_dzero, xjjroot::min_hist_dzero, xjjroot::max_hist_dzero);
  TH1F* hmass_incl_bkg = new TH1F("hmass_incl_bkg", ";m_{K#pi} (GeV/c);", xjjroot::n_hist_dzero, xjjroot::min_hist_dzero, xjjroot::max_hist_dzero);
  TH1F* hmass_incl_raw_unweight = new TH1F("hmass_incl_raw_unweight", ";m_{K#pi} (GeV/c);", xjjroot::n_hist_dzero, xjjroot::min_hist_dzero, xjjroot::max_hist_dzero);
  TH1F* hmass_incl_bkg_unweight = new TH1F("hmass_incl_bkg_unweight", ";m_{K#pi} (GeV/c);", xjjroot::n_hist_dzero, xjjroot::min_hist_dzero, xjjroot::max_hist_dzero);
  TH1F* hnphoton = new TH1F("hnphoton", ";;", 2, 0, 2);
  int nentries = f->GetEntries();
  int passevtraw = 0, passevtbkg = 0, passevthlt = 0, passevtphoki = 0, passevtqual = 0;
  for(int i=0; i<nentries; i++)
    {
      if(i%10000==0 || i==nentries-1) xjjc::progressbar(i, nentries);
      f->GetEntry(i);

      // event selection + hlt
      if(pa.ishi() && (etr->hiBin() < pa["centmin"]*2 || etr->hiBin() > pa["centmax"]*2)) continue;
      if(!etr->presel(ignorehlt)) continue;

      passevthlt++;

      // photon selection
      int jlead = -1, passki = 0;
      for(int j=0; j<ptr->nPho(); j++) 
        {
          // pt, eta
          if(ptr->val<float>("phoEt", j) <= pa["phoptmin"] || fabs(ptr->val<float>("phoSCEta", j)) >= pa["phoetamax"]) continue;
          passki++;
          // hovere
          if(!ptr->presel(j)) continue;
          jlead = j;
          break;
        }
      if(passki) passevtphoki++;
      if(jlead < 0) continue;

      if(passki) passevtqual++;

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
          if(!dtr->tightsel(j)) continue;

          // std::cout<<etr->hiBin()<<std::endl;
          float effweight = ebin.geteff(dtr->val<float>("Dpt", j), fabs(dtr->val<float>("Dy", j)), etr->hiBin());

          // dphi calculation
          float dphi = phoD::cal_dphi_01(dtr->val<float>("Dphi", j), 
                                         ptr->val<float>("phoPhi", jlead)); // 0 ~ 1
          int ibin = vb.ibin(dphi);
          if(see_raw)
            {
              hmass_incl_raw->Fill(dtr->val<float>("Dmass", j), 1./effweight);
              hmass_raw[ibin]->Fill(dtr->val<float>("Dmass", j), 1./effweight);
              hmass_incl_raw_unweight->Fill(dtr->val<float>("Dmass", j));
              hmass_raw_unweight[ibin]->Fill(dtr->val<float>("Dmass", j));
            }
          if(see_bkg)
            {
              hmass_incl_bkg->Fill(dtr->val<float>("Dmass", j), 1./effweight);
              hmass_bkg[ibin]->Fill(dtr->val<float>("Dmass", j), 1./effweight);
              hmass_incl_bkg_unweight->Fill(dtr->val<float>("Dmass", j));
              hmass_bkg_unweight[ibin]->Fill(dtr->val<float>("Dmass", j));
            }
        }
    }
  xjjc::progressbar_summary(nentries);
  std::cout<<"Events passing HLT + event filter: \e[31m"<<passevthlt<<"\e[0m."<<std::endl;
  std::cout<<"Events with passed-kinematic photons: \e[31m"<<passevtphoki<<"\e[0m."<<std::endl;
  std::cout<<"Events with photons passing hovere selection: \e[31m"<<passevtqual<<"\e[0m."<<std::endl;
  std::cout<<"Events with raw photons: \e[31m"<<passevtraw<<"\e[0m."<<std::endl;
  std::cout<<"Events with decay photons: \e[31m"<<passevtbkg<<"\e[0m."<<std::endl;
  hnphoton->SetBinContent(1, passevtraw);
  hnphoton->SetBinContent(2, passevtbkg);

  std::string outputname = "rootfiles/" + outsubdir + "_" + pa.tag() + "/savehist.root";
  xjjroot::mkdir(outputname);
  TFile* outf = new TFile(outputname.c_str(), "recreate");
  xjjroot::writehist(hmass_incl_raw, 10);
  xjjroot::writehist(hmass_incl_bkg, 10);
  xjjroot::writehist(hmass_incl_raw_unweight, 10);
  xjjroot::writehist(hmass_incl_bkg_unweight, 10);
  for(auto& hh : hmass_raw) { xjjroot::writehist(hh, 10); }
  for(auto& hh : hmass_bkg) { xjjroot::writehist(hh, 10); }
  for(auto& hh : hmass_raw_unweight) { xjjroot::writehist(hh, 10); }
  for(auto& hh : hmass_bkg_unweight) { xjjroot::writehist(hh, 10); }
  xjjroot::writehist(hnphoton, 10);
  pa.write();
  outf->Close();

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc==12)
    {
      phoD::param pa(atoi(argv[3]), atof(argv[4]), atof(argv[5]), atof(argv[6]), atof(argv[7]), atof(argv[8]), atof(argv[9]), atof(argv[10]), atoi(argv[11]));
      return pdana_savehist(argv[1], argv[2], pa);
    }
  return 1;
}
