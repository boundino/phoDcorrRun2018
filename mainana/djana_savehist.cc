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

int djana_savehist(std::string inputname, std::string outsubdir, Djet::param& pa)
{
  TFile* inf = TFile::Open(inputname.c_str());
  phoD::forest* f = new phoD::forest(inf, pa.ishi());
  phoD::etree* etr = f->etr();
  phoD::dtree* dtr = f->dtr();
  phoD::jtree* jtr = f->jtr();
  bool ignorehlt = xjjc::str_contains(inputname, "Bias");

  eff::effbins ebin(pa.ishi()?"effrootfiles/eff_PbPb_prompt.root":"effrootfiles/eff_pp_prompt.root", pa.ishi());

  phoD::bins<float> vb(pa.ishi()?Djet::bins_dphi_aa:Djet::bins_dphi_pp);
  std::vector<TH1F*> hmass_raw_fitweigh(vb.n(), 0), hmass_raw_unweight(vb.n(), 0);
  for(int k=0; k<vb.n(); k++)
    {
      hmass_raw_fitweigh[k] = new TH1F(Form("hmass_raw_fitweigh_%d", k), 
                                       Form("#Delta#phi/#pi: %s - %s;m_{K#pi} (GeV/c);", xjjc::number_remove_zero(vb[k]).c_str(), xjjc::number_remove_zero(vb[k+1]).c_str()), 
                                       xjjroot::n_hist_dzero, xjjroot::min_hist_dzero, xjjroot::max_hist_dzero);
      hmass_raw_unweight[k] = new TH1F(Form("hmass_raw_unweight_%d", k), 
                                       Form("#Delta#phi/#pi: %s - %s;m_{K#pi} (GeV/c);", xjjc::number_remove_zero(vb[k]).c_str(), xjjc::number_remove_zero(vb[k+1]).c_str()), 
                                       xjjroot::n_hist_dzero, xjjroot::min_hist_dzero, xjjroot::max_hist_dzero);
    }
  TH1F* hmass_incl_raw_fitweigh = new TH1F("hmass_incl_raw_fitweigh", ";m_{K#pi} (GeV/c);", xjjroot::n_hist_dzero, xjjroot::min_hist_dzero, xjjroot::max_hist_dzero);
  TH1F* hmass_incl_raw_unweight = new TH1F("hmass_incl_raw_unweight", ";m_{K#pi} (GeV/c);", xjjroot::n_hist_dzero, xjjroot::min_hist_dzero, xjjroot::max_hist_dzero);
  TH1F* heff_raw = new TH1F("heff_raw", ";#Delta#phi^{jD} / #pi;1 / #alpha #times #epsilon", vb.n(), vb.v().data());
  TH1F* heffnorm_raw = new TH1F("heffnorm_raw", ";#Delta#phi^{jD} / #pi;1 / #alpha #times #epsilon", vb.n(), vb.v().data());
  TH1F* hnjet = new TH1F("hnjet", ";;", 1, 0, 1);
  std::map<std::string, TH1F*> hd;
  hd["pt_raw"] = new TH1F("hdpt_raw", ";D p_{T} (GeV/c);", 40, 2, 42);
  hd["pt_bkg"] = new TH1F("hdpt_bkg", ";D p_{T} (GeV/c);", 40, 2, 42);
  int nentries = f->GetEntries();
  int passevtraw = 0, passevthlt = 0, passevtjetki = 0, passevtqual = 0;
  for(int i=0; i<nentries; i++)
    {
      if(i%10000==0 || i==nentries-1) xjjc::progressbar(i, nentries);
      f->GetEntry(i);

      // event selection + hlt
      if(!etr->hltsel_jet()) continue;
      if(pa.ishi() && (etr->hiBin() < pa["centmin"]*2 || etr->hiBin() > pa["centmax"]*2)) continue;
      if(!etr->evtsel()) continue;

      passevthlt++;

      // jet selection
      int jlead = -1, passki = 0;
      for(int j=0; j<jtr->nref(); j++) 
        {
          // pt, eta
          if((*jtr)["jtpt"][j] <= pa["jtptmin"] || fabs((*jtr)["jteta"][j]) >= pa["jtetamax"]) continue;
          passki++;
          // other jet selections ?
          // if(!jtr->presel(j)) continue;
          jlead = j;
          break;
        }
      if(passki) passevtjetki++;
      if(jlead < 0) continue;

      passevtqual++;

      // D mesons
      for(int j=0; j<dtr->Dsize(); j++)
        {
          if(!dtr->presel(j) || !dtr->tightsel(j)) continue;
          // D kinematic 
          if(fabs((*dtr)["Dy"][j]) < pa["Dymax"])
            {
              if(pdg::dzero_sigreg((*dtr)["Dmass"][j])) 
                hd["pt_raw"]->Fill((*dtr)["Dpt"][j]);
              if(pdg::dzero_sideband((*dtr)["Dmass"][j])) 
                hd["pt_bkg"]->Fill((*dtr)["Dpt"][j]);
            }
          // D selection applied in skim
          if((*dtr)["Dpt"][j] < pa["Dptmin"] || (*dtr)["Dpt"][j] > pa["Dptmax"]) continue;
          if(fabs((*dtr)["Dy"][j]) > pa["Dymax"]) continue;

          float effweight = ebin.geteff((*dtr)["Dpt"][j], fabs((*dtr)["Dy"][j]), etr->hiBin());
          if(effweight<1.e-6) { std::cout<<effweight<<": "<<(*dtr)["Dpt"][j]<<", "<<fabs((*dtr)["Dy"][j])<<", "<<etr->hiBin()<<std::endl; continue; }
          // dphi calculation
          float dphi = phoD::cal_dphi_01((*dtr)["Dphi"][j], 
                                         (*jtr)["jtphi"][jlead]); // 0 ~ 1
          int ibin = vb.ibin(dphi);
          hmass_incl_raw_fitweigh->Fill((*dtr)["Dmass"][j], 1./effweight);
          hmass_raw_fitweigh[ibin]->Fill((*dtr)["Dmass"][j], 1./effweight);
          hmass_incl_raw_unweight->Fill((*dtr)["Dmass"][j]);
          hmass_raw_unweight[ibin]->Fill((*dtr)["Dmass"][j]);
          if(pdg::dzero_sigreg((*dtr)["Dmass"][j]))
            {
              heff_raw->Fill(dphi, 1./effweight);
              heffnorm_raw->Fill(dphi);
            }
        }
    }
  xjjc::progressbar_summary(nentries);
  std::cout<<"Events passing HLT + event filter: \e[31m"<<passevthlt<<"\e[0m."<<std::endl;
  std::cout<<"Events with passed-kinematic jets: \e[31m"<<passevtjetki<<"\e[0m."<<std::endl;
  hnjet->SetBinContent(1, passevtqual);

  for(int i=0; i<vb.n(); i++)
    {
      heff_raw->SetBinContent(i+1, heff_raw->GetBinContent(i+1)/heffnorm_raw->GetBinContent(i+1));
      heff_raw->SetBinError(i+1, heff_raw->GetBinError(i+1)/heffnorm_raw->GetBinContent(i+1));
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
  xjjroot::writehist(hmass_incl_raw_unweight, 10);
  for(auto& hh : hmass_raw_fitweigh) { xjjroot::writehist(hh, 10); }
  for(auto& hh : hmass_raw_unweight) { xjjroot::writehist(hh, 10); }
  xjjroot::writehist(hnjet, 10);
  xjjroot::writehist(heff_raw, 10);
  pa.write();
  outf->Close();

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc==12)
    {
      Djet::param pa(atoi(argv[3]), atof(argv[4]), atof(argv[5]), atof(argv[6]), atof(argv[7]), atof(argv[8]), atof(argv[9]), atof(argv[10]), atoi(argv[11]));
      return djana_savehist(argv[1], argv[2], pa);
    }
  return 1;
}
