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
  TFile* inf_fine = TFile::Open(inputname.c_str());
  TFile* inf_less = TFile::Open(xjjc::str_replaceall(inputname, "finebin", "lessbin").c_str());
  phoD::param pa(inf_fine);

  TH1F* hdphi_raw_finebin = (TH1F*)inf_fine->Get("hdphi_raw");
  hdphi_raw_finebin->SetName("hdphi_raw_finebine");
  TH1F* hdphi_raw_lessbin = (TH1F*)inf_less->Get("hdphi_raw");
  hdphi_raw_lessbin->SetName("hdphi_raw_lessbin");

  hdphi_raw_finebin->SetMinimum(0);
  hdphi_raw_finebin->SetMaximum(hdphi_raw_finebin->GetMaximum()*1.4);
  hdphi_raw_finebin->GetXaxis()->SetNdivisions(-505);
  xjjroot::sethempty(hdphi_raw_finebin, 0, 0.3);
  xjjroot::setthgrstyle(hdphi_raw_finebin, kGray, 20, 1, kGray, 1, 2);
  hdphi_raw_lessbin->SetMinimum(0);
  hdphi_raw_lessbin->SetMaximum(hdphi_raw_lessbin->GetMaximum()*1.4);
  hdphi_raw_lessbin->GetXaxis()->SetNdivisions(-505);
  xjjroot::sethempty(hdphi_raw_lessbin, 0, 0.3);
  xjjroot::setthgrstyle(hdphi_raw_lessbin, kBlack, 20, 1, kBlack, 1, 2);

  xjjroot::setgstyle(1);
  TCanvas* c = new TCanvas("c", "", 600, 600);
  hdphi_raw_finebin->Draw("pe");
  hdphi_raw_lessbin->Draw("pe same");
  pa.drawtex(0.23, 0.85, 0.035);
  xjjroot::drawCMSleft();
  xjjroot::drawCMSright(Form("%s #sqrt{s_{NN}} = 5.02 TeV", pa.tag("ishi").c_str()));
  std::string output = "plots/" + outsubdir + "_" + pa.tag() + "/chdphi_raw_bin.pdf";
  xjjroot::mkdir(output);
  std::cout<<"\e[0m";
  c->SaveAs(output.c_str());

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc==3)
    return pdana_drawhist(argv[1], argv[2]);
  return 1;
}
