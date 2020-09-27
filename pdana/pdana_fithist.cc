#include <TFile.h>
#include <TCanvas.h>
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
  phoD::bins<float> vb(phoD::bins_dphi);
  std::vector<TH1F*> hmass(vb.n(), 0);
  for(int k=0; k<vb.n(); k++) { hmass[k] = (TH1F*)inf->Get(Form("hmass_%d", k)); }
  TH1F* hmass_incl = (TH1F*)inf->Get("hmass_incl");
  TH1F* hdphi_raw = new TH1F("hdphi_raw", ";#Delta#phi^{#gammaD} / #pi;dN^{#gammaD}/d#phi", vb.n(), vb.v().data());
  TFile* inftpl = TFile::Open("masstpl_PbPb.root");
  TH1F* hmassmc_signal = (TH1F*)inftpl->Get("hHistoRMassSignal_pt_0_dr_0");
  TH1F* hmassmc_swapped = (TH1F*)inftpl->Get("hHistoRMassSwapped_pt_0_dr_0");
  xjjroot::dfitter df("YCF");
  std::string fitoutput = "plots/" + outsubdir + "_" + pa.tag() + "/idx/cmass";
  xjjroot::mkdir(fitoutput);
  df.fit(hmass_incl, hmassmc_signal, hmassmc_swapped, pa.tag("ishi").c_str(), Form("%s_incl", fitoutput.c_str()), 
         std::vector<TString>({pa.tag("pt").c_str(), pa.tag("y").c_str()}));
  for(int k=0; k<vb.n(); k++)
    {
      df.fit(hmass[k], hmassmc_signal, hmassmc_swapped, pa.tag("ishi").c_str(), Form("%s_%d", fitoutput.c_str(), k), 
             std::vector<TString>({pa.tag("pt").c_str(), pa.tag("y").c_str(), vb.tag(k, "#Delta#phi/#pi").c_str()}));
      hdphi_raw->SetBinContent(k+1, df.GetY()/vb.width(k)/M_PI);
      hdphi_raw->SetBinError(k+1, df.GetYE()/vb.width(k)/M_PI);
    }
  std::string outputname = "rootfiles/" + outsubdir + "_" + pa.tag() + "/fithist.root";
  xjjroot::mkdir(outputname);
  TFile* outf = new TFile(outputname.c_str(), "recreate");
  hdphi_raw->Write();
  pa.write();
  outf->Close();
 
  xjjroot::setgstyle(1);
  hdphi_raw->SetMinimum(0);
  hdphi_raw->GetXaxis()->SetNdivisions(-505);
  xjjroot::sethempty(hdphi_raw, 0, 0.3);
  xjjroot::setthgrstyle(hdphi_raw, kBlack, 20, 1, kBlack, 1, 2);
  TCanvas* c = new TCanvas("c", "", 600, 600);
  hdphi_raw->Draw("pe");
  pa.drawtex(0.23, 0.85, 0.035);
  xjjroot::drawCMSleft();
  xjjroot::drawCMSright(Form("%s #sqrt{s_{NN}} = 5.02 TeV", pa.tag("ishi").c_str()));
  std::string coutput = "plots/" + outsubdir + "_" + pa.tag() + "/chdphi_raw.pdf";
  c->SaveAs(coutput.c_str());

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc==3)
    {
      return pdana_fithist(argv[1], argv[2]);
    }
  return 1;
}
