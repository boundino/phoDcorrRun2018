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
#include "pdg.h"

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
  std::vector<TH1F*> hmass_raw_fitweigh(vb.n(), 0), hmass_bkg_fitweigh(vb.n(), 0), hmass_raw_unweight(vb.n(), 0), hmass_bkg_unweight(vb.n(), 0);
  for(int k=0; k<vb.n(); k++)
    {
      hmass_raw_fitweigh[k] = new TH1F(Form("hmass_raw_fitweigh_%d", k), 
                                       Form("#Delta#phi/#pi: %s - %s;m_{K#pi} (GeV/c);", xjjc::number_remove_zero(vb[k]).c_str(), xjjc::number_remove_zero(vb[k+1]).c_str()), 
                                       xjjroot::n_hist_dzero, xjjroot::min_hist_dzero, xjjroot::max_hist_dzero);
      hmass_bkg_fitweigh[k] = new TH1F(Form("hmass_bkg_fitweigh_%d", k), 
                                       Form("#Delta#phi/#pi: %s - %s;m_{K#pi} (GeV/c);", xjjc::number_remove_zero(vb[k]).c_str(), xjjc::number_remove_zero(vb[k+1]).c_str()), 
                                       xjjroot::n_hist_dzero, xjjroot::min_hist_dzero, xjjroot::max_hist_dzero);
      hmass_raw_unweight[k] = new TH1F(Form("hmass_raw_unweight_%d", k), 
                                       Form("#Delta#phi/#pi: %s - %s;m_{K#pi} (GeV/c);", xjjc::number_remove_zero(vb[k]).c_str(), xjjc::number_remove_zero(vb[k+1]).c_str()), 
                                       xjjroot::n_hist_dzero, xjjroot::min_hist_dzero, xjjroot::max_hist_dzero);
      hmass_bkg_unweight[k] = new TH1F(Form("hmass_bkg_unweight_%d", k), 
                                       Form("#Delta#phi/#pi: %s - %s;m_{K#pi} (GeV/c);", xjjc::number_remove_zero(vb[k]).c_str(), xjjc::number_remove_zero(vb[k+1]).c_str()), 
                                       xjjroot::n_hist_dzero, xjjroot::min_hist_dzero, xjjroot::max_hist_dzero);
    }
  TH1F* hmass_incl_raw_fitweigh = new TH1F("hmass_incl_raw_fitweigh", ";m_{K#pi} (GeV/c);", xjjroot::n_hist_dzero, xjjroot::min_hist_dzero, xjjroot::max_hist_dzero);
  TH1F* hmass_incl_bkg_fitweigh = new TH1F("hmass_incl_bkg_fitweigh", ";m_{K#pi} (GeV/c);", xjjroot::n_hist_dzero, xjjroot::min_hist_dzero, xjjroot::max_hist_dzero);
  TH1F* hmass_incl_raw_unweight = new TH1F("hmass_incl_raw_unweight", ";m_{K#pi} (GeV/c);", xjjroot::n_hist_dzero, xjjroot::min_hist_dzero, xjjroot::max_hist_dzero);
  TH1F* hmass_incl_bkg_unweight = new TH1F("hmass_incl_bkg_unweight", ";m_{K#pi} (GeV/c);", xjjroot::n_hist_dzero, xjjroot::min_hist_dzero, xjjroot::max_hist_dzero);
  TH1F* heff_raw = new TH1F("heff_raw", ";#Delta#phi^{#gammaD} / #pi;1 / #alpha #times #epsilon", vb.n(), vb.v().data());
  TH1F* heff_bkg = new TH1F("heff_bkg", ";#Delta#phi^{#gammaD} / #pi;1 / #alpha #times #epsilon", vb.n(), vb.v().data());
  TH1F* heffnorm_raw = new TH1F("heffnorm_raw", ";#Delta#phi^{#gammaD} / #pi;1 / #alpha #times #epsilon", vb.n(), vb.v().data());
  TH1F* heffnorm_bkg = new TH1F("heffnorm_bkg", ";#Delta#phi^{#gammaD} / #pi;1 / #alpha #times #epsilon", vb.n(), vb.v().data());
  TH1F* hnphoton = new TH1F("hnphoton", ";;", 2, 0, 2);
  std::map<std::string, TH1F*> hd;
  hd["pt_raw"] = new TH1F("hdpt_raw", ";D p_{T} (GeV/c);", 40, 2, 42);
  hd["pt_bkg"] = new TH1F("hdpt_bkg", ";D p_{T} (GeV/c);", 40, 2, 42);
  int nentries = f->GetEntries();
  int passevtraw = 0, passevtbkg = 0, passevthlt = 0, passevtphoki = 0, passevtqual = 0;
  for(int i=0; i<nentries; i++)
    {
      xjjc::progressslide(i, nentries, 10000);
      f->GetEntry(i);

      // event selection + hlt
      if(pa.ishi() && (etr->hiBin() < pa["centmin"]*2 || etr->hiBin() > pa["centmax"]*2)) continue;
      if(!etr->hltsel_photon() && !ignorehlt) continue;
      if(!etr->evtsel()) continue;

      passevthlt++;

      // photon selection
      int jlead = -1, passki = 0;
      for(int j=0; j<ptr->nPho(); j++) 
        {
          // pt, eta
          if((*ptr)["phoEt"][j] <= pa["phoptmin"] || fabs((*ptr)["phoSCEta"][j]) >= pa["phoetamax"]) continue;
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
          if(!dtr->presel(j)) continue;
          if(!dtr->tightsel(j)) continue;
          if(see_raw)
            {
              // D kinematic
              if(fabs((*dtr)["Dy"][j]) < pa["Dymax"])
                {
                  if(pdg::dzero_sigreg((*dtr)["Dmass"][j]))
                    hd["pt_raw"]->Fill((*dtr)["Dpt"][j]);
                  if(pdg::dzero_sideband((*dtr)["Dmass"][j]))
                    hd["pt_bkg"]->Fill((*dtr)["Dpt"][j]);
                }
            }
          if((*dtr)["Dpt"][j] < pa["Dptmin"] || (*dtr)["Dpt"][j] > pa["Dptmax"]) continue;
          if(fabs((*dtr)["Dy"][j]) > pa["Dymax"]) continue;

          // std::cout<<etr->hiBin()<<std::endl;
          float effweight = ebin.geteff((*dtr)["Dpt"][j], fabs((*dtr)["Dy"][j]), etr->hiBin());

          // dphi calculation
          float dphi = phoD::cal_dphi_0pi((*dtr)["Dphi"][j], 
                                         (*ptr)["phoPhi"][jlead]); // 0 ~ 1
          int ibin = vb.ibin(dphi);
          if(see_raw)
            {
              hmass_incl_raw_fitweigh->Fill((*dtr)["Dmass"][j], 1./effweight);
              hmass_raw_fitweigh[ibin]->Fill((*dtr)["Dmass"][j], 1./effweight);
              hmass_incl_raw_unweight->Fill((*dtr)["Dmass"][j]);
              hmass_raw_unweight[ibin]->Fill((*dtr)["Dmass"][j]);
              if((*dtr)["Dmass"][j] > pdg::DZERO_MASS-0.045 && (*dtr)["Dmass"][j] < pdg::DZERO_MASS+0.045)
                {
                  heff_raw->Fill(dphi, 1./effweight);
                  heffnorm_raw->Fill(dphi);
                }
            }
          if(see_bkg)
            {
              hmass_incl_bkg_fitweigh->Fill((*dtr)["Dmass"][j], 1./effweight);
              hmass_bkg_fitweigh[ibin]->Fill((*dtr)["Dmass"][j], 1./effweight);
              hmass_incl_bkg_unweight->Fill((*dtr)["Dmass"][j]);
              hmass_bkg_unweight[ibin]->Fill((*dtr)["Dmass"][j]);
              if((*dtr)["Dmass"][j] > pdg::DZERO_MASS-0.045 && (*dtr)["Dmass"][j] < pdg::DZERO_MASS+0.045)
                {
                  heff_bkg->Fill(dphi, 1./effweight);
                  heffnorm_bkg->Fill(dphi);
                }
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

  for(int i=0; i<vb.n(); i++)
    {
      heff_raw->SetBinContent(i+1, heff_raw->GetBinContent(i+1)/heffnorm_raw->GetBinContent(i+1));
      heff_raw->SetBinError(i+1, heff_raw->GetBinError(i+1)/heffnorm_raw->GetBinContent(i+1));
      heff_bkg->SetBinContent(i+1, heff_bkg->GetBinContent(i+1)/heffnorm_bkg->GetBinContent(i+1));
      heff_bkg->SetBinError(i+1, heff_bkg->GetBinError(i+1)/heffnorm_bkg->GetBinContent(i+1));
    }

  hd["pt_raw"]->Sumw2();
  hd["pt_bkg"]->Sumw2();
  hd["pt_bkg"]->Scale(pdg::DZERO_SCALE);
  hd["pt_sig"] = (TH1F*)hd["pt_raw"]->Clone("hdpt_sig");
  hd["pt_sig"]->Add(hd["pt_bkg"], -1);

  std::string outputname = "rootfiles/" + outsubdir + "_" + pa.tag() + "/savehist.root";
  xjjroot::mkdir(outputname);
  TFile* outf = new TFile(outputname.c_str(), "recreate");
  for(auto& hh : hd) { xjjroot::writehist(hh.second, 10); }
  xjjroot::writehist(hmass_incl_raw_fitweigh, 10);
  xjjroot::writehist(hmass_incl_bkg_fitweigh, 10);
  xjjroot::writehist(hmass_incl_raw_unweight, 10);
  xjjroot::writehist(hmass_incl_bkg_unweight, 10);
  for(auto& hh : hmass_raw_fitweigh) { xjjroot::writehist(hh, 10); }
  for(auto& hh : hmass_bkg_fitweigh) { xjjroot::writehist(hh, 10); }
  for(auto& hh : hmass_raw_unweight) { xjjroot::writehist(hh, 10); }
  for(auto& hh : hmass_bkg_unweight) { xjjroot::writehist(hh, 10); }
  xjjroot::writehist(hnphoton, 10);
  xjjroot::writehist(heff_raw, 10);
  xjjroot::writehist(heff_bkg, 10);
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
