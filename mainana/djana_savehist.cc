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

int djana_savehist(std::string inputname, std::string outsubdir, Djet::param& pa, bool isleadingjet = false)
{
  TFile* inf = TFile::Open(inputname.c_str());
  phoD::forest* f = new phoD::forest(inf, pa.ishi());
  phoD::etree* etr = f->etr();
  phoD::dtree* dtr = f->dtr();
  phoD::jtree* jtr = f->jtr();
  bool ignorehlt = xjjc::str_contains(inputname, "Bias");

  eff::effbins ebin(pa.ishi()?"effrootfiles/eff_PbPb_prompt.root":"effrootfiles/eff_pp_prompt.root", pa.ishi());
  std::vector<std::string> type = {"fitweigh", "unweight"};

  std::map<std::string, xjjana::bins<float>> vb;
  std::map<std::string, std::vector<TH1F*>> hmass;
  std::map<std::string, TH1F*> hmass_incl, heff, heffnorm;
  for(auto& v : Djet::var) // dphi, dr
    {
      vb[v] = xjjana::bins<float>(pa.ishi()?Djet::bins_aa[v]:Djet::bins_pp[v]);
      heff[v] = new TH1F(Form("heff_%s", v.c_str()), Form(";%s;1 / #alpha #times #epsilon", Djet::vartex[v].c_str()), vb[v].n(), vb[v].v().data());
      heffnorm[v] = new TH1F(Form("heffnorm_%s", v.c_str()), Form(";%s;1 / #alpha #times #epsilon", Djet::vartex[v].c_str()), vb[v].n(), vb[v].v().data());
      for(auto& t : type) // fitweight, unweight
        {
          hmass_incl[v+"_"+t] = new TH1F(Form("hmass_incl_%s_%s", v.c_str(), t.c_str()), ";m_{K#pi} (GeV/c);", xjjroot::n_hist_dzero, xjjroot::min_hist_dzero, xjjroot::max_hist_dzero);
          for(int k=0; k<vb[v].n(); k++)
            {
              TH1F* h =  new TH1F(Form("hmass_%s_%s_%d", v.c_str(), t.c_str(), k),
                                  Form("%s: %s - %s;m_{K#pi} (GeV/c);", Djet::vartex[v].c_str(), xjjc::number_remove_zero(vb[v][k]).c_str(), xjjc::number_remove_zero(vb[v][k+1]).c_str()),
                                  xjjroot::n_hist_dzero, xjjroot::min_hist_dzero, xjjroot::max_hist_dzero);
              hmass[v+"_"+t].push_back(h);
            }
        }
    }
  TH1F* hnjet = new TH1F("hnjet", ";;", 1, 0, 1);

  int nentries = f->GetEntries();
  int passevtraw = 0, passevthlt = 0, passevtjetki = 0, njet = 0;
  int effbug = 0;
  for(int i=0; i<nentries; i++)
    {
      xjjc::progressslide(i, nentries, 100000);
      f->GetEntry(i);

      // event selection + hlt
      if(!etr->hltsel_jet()) continue;
      if(pa.ishi() && (etr->hiBin() < pa["centmin"]*2 || etr->hiBin() > pa["centmax"]*2)) continue;
      if(!etr->evtsel()) continue;

      passevthlt++;

      // jet selection
      int passki = 0;
      for(int j=0; j<jtr->nref(); j++) 
        {
          // pt, eta
          if((*jtr)["jtpt"][j] <= pa["jtptmin"] || fabs((*jtr)["jteta"][j]) >= pa["jtetamax"]) continue;
          passki++;
          // other jet selections ?
          // if(!jtr->presel(j)) continue;

          njet++;

          // D mesons
          for(int k=0; k<dtr->Dsize(); k++)
            {
              // D selection
              if(!dtr->presel(k) || !dtr->tightsel(k)) continue;
              if((*dtr)["Dpt"][k] < pa["Dptmin"] || (*dtr)["Dpt"][k] > pa["Dptmax"]) continue;
              if(fabs((*dtr)["Dy"][k]) > pa["Dymax"]) continue;

              // D efficiency weight
              float effweight = ebin.geteff((*dtr)["Dpt"][k], fabs((*dtr)["Dy"][k]), etr->hiBin());
              if(effweight<1.e-6) { effbug++; continue; }

              // dphi calculation
              std::map<std::string, float> d;
              d["dphi"] = xjjana::cal_dphi_0pi((*dtr)["Dphi"][k], 
                                              (*jtr)["jtphi"][j]); // 0 ~ 1
              d["dr"] = xjjana::cal_dr((*dtr)["Dphi"][k], (*dtr)["Deta"][k], 
                                       (*jtr)["jtphi"][j], (*jtr)["jteta"][j]); //

              for(auto& v : Djet::var) // dphi, dr
                {
                  int ibin = vb[v].ibin(d[v]);
                  if(ibin < 0) continue;
                  hmass[v+"_unweight"][ibin]->Fill((*dtr)["Dmass"][k]);
                  hmass[v+"_fitweigh"][ibin]->Fill((*dtr)["Dmass"][k], 1./effweight);
                  hmass_incl[v+"_unweight"]->Fill((*dtr)["Dmass"][k]);
                  hmass_incl[v+"_fitweigh"]->Fill((*dtr)["Dmass"][k], 1./effweight);
                  if(pdg::dzero_sigreg((*dtr)["Dmass"][k]))
                    {
                      heff[v]->Fill(d[v], 1./effweight);
                      heffnorm[v]->Fill(d[v]);
                    }
                }
            }
          if(isleadingjet) break;
        }
      if(passki) passevtjetki++;
      if(effbug) break;
    }
  xjjc::progressbar_summary(nentries);

  std::cout<<"Events passing HLT + event filter: \e[31m"<<passevthlt<<"\e[0m."<<std::endl;
  std::cout<<"Events with passed-kinematic jets: \e[31m"<<passevtjetki<<"\e[0m."<<std::endl;
  hnjet->SetBinContent(1, njet);

  for(auto& v : Djet::var)
    {
      for(int i=0; i<vb[v].n(); i++)
        {
          if(heffnorm[v]->GetBinContent(i+1)==0) 
            {
              heff[v]->SetBinContent(i+1, 0);
              heff[v]->SetBinError(i+1, 0);
            }
          else
            {
              heff[v]->SetBinContent(i+1, heff[v]->GetBinContent(i+1)/heffnorm[v]->GetBinContent(i+1));
              heff[v]->SetBinError(i+1, heff[v]->GetBinError(i+1)/heffnorm[v]->GetBinContent(i+1));
            }
        }
    }

  std::string outputname = "rootfiles/" + outsubdir + "_" + pa.tag() + "/savehist.root";
  xjjroot::mkdir(outputname);
  TFile* outf = new TFile(outputname.c_str(), "recreate");
  for(auto& hh : hmass) { for(auto& h : hh.second) xjjroot::writehist(h, 10); }
  for(auto& h : hmass_incl) { xjjroot::writehist(h.second, 10); }
  for(auto& h : heff) { xjjroot::writehist(h.second, 10); }
  xjjroot::writehist(hnjet, 10);
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
