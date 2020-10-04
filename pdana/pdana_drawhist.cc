#include <TFile.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <iostream>
#include <iomanip>

#include "param.h"
#include "bins.h"

#include "xjjcuti.h"
#include "xjjrootuti.h"

int pdana_drawhist(std::string inputname, std::string outsubdir)
{
  TFile* inf = TFile::Open(inputname.c_str());
  phoD::param pa(inf);
  phoD::bins<float> vb(phoD::bins_dphi);
  TH1F* hdphi_raw = (TH1F*)inf->Get("hdphi_raw");

  hdphi_raw->SetMinimum(0);
  hdphi_raw->SetMaximum(hdphi_raw->GetMaximum()*1.4);
  hdphi_raw->GetXaxis()->SetNdivisions(-505);
  xjjroot::sethempty(hdphi_raw, 0, 0.3);
  xjjroot::setthgrstyle(hdphi_raw, kBlack, 20, 1, kBlack, 1, 2);

  xjjroot::setgstyle(1);
  TCanvas* c = new TCanvas("c", "", 600, 600);
  hdphi_raw->Draw("pe");
  pa.drawtex(0.23, 0.85, 0.035);
  xjjroot::drawCMSleft();
  xjjroot::drawCMSright(Form("%s #sqrt{s_{NN}} = 5.02 TeV", pa.tag("ishi").c_str()));
  std::string output = "plots/" + outsubdir + "_" + pa.tag() + "/chdphi_raw.pdf";
  xjjroot::mkdir(output);
  c->SaveAs(output.c_str());

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc==3)
    return pdana_drawhist(argv[1], argv[2]);
  return 1;
}
