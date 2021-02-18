#include <TFile.h>
#include <TCanvas.h>
#include <TH2F.h>
#include <TH1F.h>
#include <iostream>
#include <vector>

#include "xjjcuti.h"
#include "xjjrootuti.h"
#include "eff.h"

int eff_drawhist(std::string inputname, bool ishi)
{
  TFile* inf = TFile::Open(Form("rootfiles/%s.root", inputname.c_str()));

  int col[] = {
    kBlack,
    xjjroot::mycolor_middle["azure"], 
    xjjroot::mycolor_middle["orange"], 
    xjjroot::mycolor_middle["green"]
  };

  eff::effbins ebin(ishi);
  std::vector<TLegend*> leg(ebin.ncent());
  for(auto& ll : leg) 
    { 
      ll = new TLegend(0.65, 0.84-0.040*ebin.ny(), 0.90, 0.84);
      xjjroot::setleg(ll, 0.035);
    }
  std::vector<TH1F*> heff(ebin.nycent(), 0);
  for(int k=0; k<ebin.nycent(); k++)
    {
      int iy = ebin.index(k)[0];
      int icent = ebin.index(k)[1];
      heff[k] = (TH1F*)inf->Get(Form("heff_%d", k));
      heff[k]->SetMinimum(0);
      heff[k]->SetMaximum(1);
      xjjroot::setthgrstyle(heff[k], col[iy], 20, 1., col[iy], 1, 1);
      leg[icent]->AddEntry(heff[k], ebin.label(k)[0].c_str(), "pl");
    }

  TH2F* hempty = new TH2F("hempty", ";p_{T} (GeV/c);#alpha #times #epsilon", 10, 0, 30, 10, 0, 1);
  xjjroot::sethempty(hempty, 0, 0.1);

  xjjroot::setgstyle(1);
  TCanvas* c = new TCanvas("c", "", 600*ebin.ncent(), 600);
  c->Divide(ebin.ncent(), 1);
  for(int k=0; k<ebin.ncent(); k++)
    {
      c->cd(k+1);
      hempty->Draw();
      xjjroot::drawCMSleft("Simulation");
      xjjroot::drawCMSright(Form("%s #sqrt{s_{NN}} = 5.02 TeV", ishi?"PbPb":"pp"));
      leg[k]->Draw();
      xjjroot::drawtex(0.25, 0.82, ebin.label(k)[1].c_str(), 0.035);
    }
  for(int k=0; k<ebin.nycent(); k++)
    {
      int icent = ebin.index(k)[1];
      c->cd(icent+1);
      heff[k]->Draw("ple same");   
    }

  std::string output = "plots/" + inputname + "/eff.pdf";
  xjjroot::mkdir(output);
  c->SaveAs(output.c_str());
  return 0;
}

int main(int argc, char* argv[])
{
  if(argc==3)
    {
      return eff_drawhist(argv[1], atoi(argv[2]));
    }
  return 1;
}
