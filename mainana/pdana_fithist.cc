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
  std::vector<TH1F*> hmass_raw(vb.n(), 0), hmass_bkg(vb.n(), 0);
  for(int k=0; k<vb.n(); k++) { hmass_raw[k] = (TH1F*)inf->Get(Form("hmass_raw_%d", k)); }
  for(int k=0; k<vb.n(); k++) { hmass_bkg[k] = (TH1F*)inf->Get(Form("hmass_bkg_%d", k)); }
  TH1F* hmass_incl_raw = (TH1F*)inf->Get("hmass_incl_raw");
  TH1F* hmass_incl_bkg = (TH1F*)inf->Get("hmass_incl_bkg");
  TH1F* hdphi_raw = new TH1F("hdphi_raw", ";#Delta#phi^{#gammaD} / #pi;dN^{#gammaD}/d#phi", vb.n(), vb.v().data());
  TH1F* hdphi_bkg = new TH1F("hdphi_bkg", ";#Delta#phi^{#gammaD} / #pi;dN^{#gammaD}/d#phi", vb.n(), vb.v().data());
  TH1F* hnphoton = (TH1F*)inf->Get("hnphoton");
  float nphoton_raw = hnphoton->GetBinContent(1);
  float nphoton_bkg = hnphoton->GetBinContent(2);

  TFile* inftpl = TFile::Open("masstpl_PbPb.root");
  TH1F* hmassmc_signal = (TH1F*)inftpl->Get("hHistoRMassSignal_pt_0_dr_0");
  TH1F* hmassmc_swapped = (TH1F*)inftpl->Get("hHistoRMassSwapped_pt_0_dr_0");

  xjjroot::dfitter df("YCF");
  std::string fitoutput = "plots/" + outsubdir + "_" + pa.tag() + "/idx/cmass";
  xjjroot::mkdir(fitoutput);
  df.fit(hmass_incl_raw, hmassmc_signal, hmassmc_swapped, pa.tag("ishi").c_str(), Form("%s_incl_raw", fitoutput.c_str()), 
         std::vector<TString>({pa.tag("pt").c_str(), pa.tag("y").c_str()}));
  df.fit(hmass_incl_bkg, hmassmc_signal, hmassmc_swapped, pa.tag("ishi").c_str(), Form("%s_incl_bkg", fitoutput.c_str()), 
         std::vector<TString>({pa.tag("pt").c_str(), pa.tag("y").c_str()}));
  for(int k=0; k<vb.n(); k++)
    {
      df.fit(hmass_raw[k], hmassmc_signal, hmassmc_swapped, pa.tag("ishi").c_str(), Form("%s_raw_%d", fitoutput.c_str(), k), 
             std::vector<TString>({pa.tag("pt").c_str(), pa.tag("y").c_str(), vb.tag(k, "#Delta#phi/#pi").c_str()}));
      hdphi_raw->SetBinContent(k+1, df.GetY()/vb.width(k)/M_PI);
      hdphi_raw->SetBinError(k+1, df.GetYE()/vb.width(k)/M_PI);

      df.fit(hmass_bkg[k], hmassmc_signal, hmassmc_swapped, pa.tag("ishi").c_str(), Form("%s_bkg_%d", fitoutput.c_str(), k), 
             std::vector<TString>({pa.tag("pt").c_str(), pa.tag("y").c_str(), vb.tag(k, "#Delta#phi/#pi").c_str()}));
      hdphi_bkg->SetBinContent(k+1, df.GetY()/vb.width(k)/M_PI);
      hdphi_bkg->SetBinError(k+1, df.GetYE()/vb.width(k)/M_PI);
    }
  hdphi_raw->SetTitle(Form("%f", nphoton_raw));
  hdphi_bkg->SetTitle(Form("%f", nphoton_bkg));

  std::string outputname = "rootfiles/" + outsubdir + "_" + pa.tag() + "/fithist.root";
  xjjroot::mkdir(outputname);
  TFile* outf = new TFile(outputname.c_str(), "recreate");
  xjjroot::writehist(hdphi_raw);
  xjjroot::writehist(hdphi_bkg);
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
