#include <TFile.h>
#include <TH1F.h>
#include <iostream>
#include <iomanip>

#include "param.h"
#include "bins.h"

#include "xjjcuti.h"
#include "xjjrootuti.h"
#include "dfitter.h"

int pdana_fithist(std::string inputname, std::string outsubdir)
{
  TFile* inf = TFile::Open(inputname.c_str());
  phoD::param pa(inf);
  phoD::bins<float> vb(pa.ishi()?phoD::bins_dphi_aa:phoD::bins_dphi_pp);
  // get hmass
  std::vector<TH1F*> hmass_raw_fitweigh(vb.n(), 0), hmass_bkg_fitweigh(vb.n(), 0), 
    hmass_raw_unweight(vb.n(), 0), hmass_bkg_unweight(vb.n(), 0);
  for(int k=0; k<vb.n(); k++)
    {
      hmass_raw_fitweigh[k] = (TH1F*)inf->Get(Form("hmass_raw_fitweigh_%d", k));
      hmass_bkg_fitweigh[k] = (TH1F*)inf->Get(Form("hmass_bkg_fitweigh_%d", k));
      hmass_raw_unweight[k] = (TH1F*)inf->Get(Form("hmass_raw_unweight_%d", k));
      hmass_bkg_unweight[k] = (TH1F*)inf->Get(Form("hmass_bkg_unweight_%d", k));
    }
  TH1F* hmass_incl_raw_fitweigh = (TH1F*)inf->Get("hmass_incl_raw_fitweigh");
  TH1F* hmass_incl_bkg_fitweigh = (TH1F*)inf->Get("hmass_incl_bkg_fitweigh");
  TH1F* hmass_incl_raw_unweight = (TH1F*)inf->Get("hmass_incl_raw_unweight");
  TH1F* hmass_incl_bkg_unweight = (TH1F*)inf->Get("hmass_incl_bkg_unweight");
  // get nphoton & heff
  TH1F* hnphoton = (TH1F*)inf->Get("hnphoton");
  float nphoton_raw = hnphoton->GetBinContent(1);
  float nphoton_bkg = hnphoton->GetBinContent(2);
  TH1F* heff_raw = (TH1F*)inf->Get("heff_raw");
  TH1F* heff_bkg = (TH1F*)inf->Get("heff_bkg");
  // get hdpt
  TH1F* hdpt_raw = (TH1F*)inf->Get("hdpt_raw");
  TH1F* hdpt_bkg = (TH1F*)inf->Get("hdpt_bkg");
  TH1F* hdpt_sig = (TH1F*)inf->Get("hdpt_sig");
  // create hdphi
  TH1F* hdphi_raw_fitweigh = new TH1F("hdphi_raw_fitweigh", ";#Delta#phi^{#gammaD} / #pi;dN^{#gammaD}/d#phi", vb.n(), vb.v().data());
  TH1F* hdphi_raw_unweight = new TH1F("hdphi_raw_unweight", ";#Delta#phi^{#gammaD} / #pi;dN^{#gammaD}/d#phi", vb.n(), vb.v().data());
  TH1F* hdphi_raw_effcorr = new TH1F("hdphi_raw_effcorr", ";#Delta#phi^{#gammaD} / #pi;dN^{#gammaD}/d#phi", vb.n(), vb.v().data());
  TH1F* hdphi_bkg_fitweigh = new TH1F("hdphi_bkg_fitweigh", ";#Delta#phi^{#gammaD} / #pi;dN^{#gammaD}/d#phi", vb.n(), vb.v().data());
  TH1F* hdphi_bkg_unweight = new TH1F("hdphi_bkg_unweight", ";#Delta#phi^{#gammaD} / #pi;dN^{#gammaD}/d#phi", vb.n(), vb.v().data());
  TH1F* hdphi_bkg_effcorr = new TH1F("hdphi_bkg_effcorr", ";#Delta#phi^{#gammaD} / #pi;dN^{#gammaD}/d#phi", vb.n(), vb.v().data());
  // get hmassmc
  TFile* inftpl = TFile::Open("masstpl_PbPb.root");
  TH1F* hmassmc_signal = (TH1F*)inftpl->Get("hHistoRMassSignal_pt_0_dr_0");
  TH1F* hmassmc_swapped = (TH1F*)inftpl->Get("hHistoRMassSwapped_pt_0_dr_0");

  xjjroot::dfitter df("YCF"), dfw("YCFW");
  std::string fitoutput = "plots/" + outsubdir + "_" + pa.tag() + "/idx/cmass";
  xjjroot::mkdir(fitoutput);
  dfw.fit(hmass_incl_raw_fitweigh, hmassmc_signal, hmassmc_swapped, pa.tag("ishi").c_str(), Form("%s_incl_raw_fitweigh", fitoutput.c_str()), 
          std::vector<TString>({pa.tag("pt").c_str(), pa.tag("y").c_str()}));
  dfw.fit(hmass_incl_bkg_fitweigh, hmassmc_signal, hmassmc_swapped, pa.tag("ishi").c_str(), Form("%s_incl_bkg_fitweigh", fitoutput.c_str()), 
          std::vector<TString>({pa.tag("pt").c_str(), pa.tag("y").c_str()}));
  df.fit(hmass_incl_raw_unweight, hmassmc_signal, hmassmc_swapped, pa.tag("ishi").c_str(), Form("%s_incl_raw_unweight", fitoutput.c_str()), 
         std::vector<TString>({pa.tag("pt").c_str(), pa.tag("y").c_str()}));
  df.fit(hmass_incl_bkg_unweight, hmassmc_signal, hmassmc_swapped, pa.tag("ishi").c_str(), Form("%s_incl_bkg_unweight", fitoutput.c_str()), 
         std::vector<TString>({pa.tag("pt").c_str(), pa.tag("y").c_str()}));
  for(int k=0; k<vb.n(); k++)
    {
      dfw.fit(hmass_raw_fitweigh[k], hmassmc_signal, hmassmc_swapped, pa.tag("ishi").c_str(), Form("%s_raw_fitweigh_%d", fitoutput.c_str(), k), 
              std::vector<TString>({pa.tag("pt").c_str(), pa.tag("y").c_str(), vb.tag(k, "#Delta#phi/#pi").c_str()}));
      hdphi_raw_fitweigh->SetBinContent(k+1, dfw.GetY()/vb.width(k));
      hdphi_raw_fitweigh->SetBinError(k+1, dfw.GetYE()/vb.width(k));

      dfw.fit(hmass_bkg_fitweigh[k], hmassmc_signal, hmassmc_swapped, pa.tag("ishi").c_str(), Form("%s_bkg_fitweigh_%d", fitoutput.c_str(), k), 
              std::vector<TString>({pa.tag("pt").c_str(), pa.tag("y").c_str(), vb.tag(k, "#Delta#phi/#pi").c_str()}));
      hdphi_bkg_fitweigh->SetBinContent(k+1, dfw.GetY()/vb.width(k));
      hdphi_bkg_fitweigh->SetBinError(k+1, dfw.GetYE()/vb.width(k));

      df.fit(hmass_raw_unweight[k], hmassmc_signal, hmassmc_swapped, pa.tag("ishi").c_str(), Form("%s_raw_unweight_%d", fitoutput.c_str(), k), 
             std::vector<TString>({pa.tag("pt").c_str(), pa.tag("y").c_str(), vb.tag(k, "#Delta#phi/#pi").c_str()}));
      hdphi_raw_unweight->SetBinContent(k+1, df.GetY()/vb.width(k));
      hdphi_raw_unweight->SetBinError(k+1, df.GetYE()/vb.width(k));
      hdphi_raw_effcorr->SetBinContent(k+1, df.GetY()/vb.width(k)*heff_raw->GetBinContent(k+1));
      hdphi_raw_effcorr->SetBinError(k+1, df.GetYE()/vb.width(k)*heff_raw->GetBinContent(k+1));

      df.fit(hmass_bkg_unweight[k], hmassmc_signal, hmassmc_swapped, pa.tag("ishi").c_str(), Form("%s_bkg_unweight_%d", fitoutput.c_str(), k), 
             std::vector<TString>({pa.tag("pt").c_str(), pa.tag("y").c_str(), vb.tag(k, "#Delta#phi/#pi").c_str()}));
      hdphi_bkg_unweight->SetBinContent(k+1, df.GetY()/vb.width(k));
      hdphi_bkg_unweight->SetBinError(k+1, df.GetYE()/vb.width(k));
      hdphi_bkg_effcorr->SetBinContent(k+1, df.GetY()/vb.width(k)*heff_bkg->GetBinContent(k+1));
      hdphi_bkg_effcorr->SetBinError(k+1, df.GetYE()/vb.width(k)*heff_bkg->GetBinContent(k+1));
    }

  hdphi_raw_fitweigh->SetTitle(Form("%f", nphoton_raw));
  hdphi_bkg_fitweigh->SetTitle(Form("%f", nphoton_bkg));
  hdphi_raw_unweight->SetTitle(Form("%f", nphoton_raw));
  hdphi_bkg_unweight->SetTitle(Form("%f", nphoton_bkg));
  hdphi_raw_effcorr->SetTitle(Form("%f", nphoton_raw));
  hdphi_bkg_effcorr->SetTitle(Form("%f", nphoton_bkg));

  std::string outputname = "rootfiles/" + outsubdir + "_" + pa.tag() + "/fithist.root";
  xjjroot::mkdir(outputname);
  TFile* outf = new TFile(outputname.c_str(), "recreate");
  xjjroot::writehist(hdphi_raw_fitweigh);
  xjjroot::writehist(hdphi_bkg_fitweigh);
  xjjroot::writehist(hdphi_raw_unweight);
  xjjroot::writehist(hdphi_bkg_unweight);
  xjjroot::writehist(hdphi_raw_effcorr);
  xjjroot::writehist(hdphi_bkg_effcorr);
  xjjroot::writehist(heff_raw);
  xjjroot::writehist(heff_bkg);
  xjjroot::writehist(hdpt_raw);
  xjjroot::writehist(hdpt_bkg);
  xjjroot::writehist(hdpt_sig);
  pa.write();
  outf->Close();

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc==3)
    return pdana_fithist(argv[1], argv[2]);
  return 1;
}
