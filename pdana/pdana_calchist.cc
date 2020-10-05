#include <TFile.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <iostream>
#include <iomanip>

#include "param.h"

#include "xjjcuti.h"
#include "xjjrootuti.h"

void seth(TH1F* h);
int pdana_calchist(std::string inputname, std::string outsubdir)
{
  TFile* inf = TFile::Open(inputname.c_str());
  phoD::param pa(inf);
  std::map<std::string, TH1F*> hdphi;
  hdphi["raw"] = (TH1F*)inf->Get("hdphi_raw");
  hdphi["bkg"] = (TH1F*)inf->Get("hdphi_bkg");

  float purity = pa.ishi()?phoD::purity_aa_:phoD::purity_pp_;
  float int_raw = hdphi["raw"]->Integral("width");
  float int_bkg = hdphi["bkg"]->Integral("width");
  hdphi["raw_scale"] = (TH1F*)hdphi["raw"]->Clone("hdphi_raw_scale");
  // hdphi["raw_scale"]->Scale(1./purity);
  hdphi["bkg_scale"] = (TH1F*)hdphi["bkg"]->Clone("hdphi_bkg_scale");
  hdphi["bkg_scale"]->Scale((1.-purity)*int_raw/int_bkg);
  hdphi["sub"] = (TH1F*)hdphi["raw_scale"]->Clone("hdphi_sub");
  hdphi["sub"]->Add(hdphi["bkg_scale"], -1);

  std::string output = "rootfiles/" + outsubdir + "_" + pa.tag() + "/calchist.root";
  xjjroot::mkdir(output);
  TFile* outf = new TFile(output.c_str(), "recreate");
  for(auto& hh : hdphi) xjjroot::writehist(hh.second);
  outf->Close();

  for(auto& hh : hdphi) seth(hh.second);
  xjjroot::setthgrstyle(hdphi["raw"], kBlack, 20, 1, kBlack, 1, 2);
  xjjroot::setthgrstyle(hdphi["bkg"], kGray, 24, 1, kGray, 1, 2);
  xjjroot::setthgrstyle(hdphi["raw_scale"], kBlack, 20, 1, kBlack, 1, 2);
  xjjroot::setthgrstyle(hdphi["bkg_scale"], kBlack, 24, 1, kBlack, 1, 2);
  xjjroot::setthgrstyle(hdphi["sub"], kBlack, 20, 1, kBlack, 1, 2);

  std::map<std::string, TLegend*> leg;
  leg["scale"] = new TLegend(0.22, 0.65-0.04*4, 0.50, 0.65);
  leg["scale"]->AddEntry((TObject*)0, Form("p = %.3f", purity), NULL);
  leg["scale"]->AddEntry(hdphi["raw_scale"], "Raw", "pl");
  leg["scale"]->AddEntry(hdphi["bkg"], "Bkg (sideband)", "pl");
  leg["scale"]->AddEntry(hdphi["bkg_scale"], "Bkg (scaled)", "pl");
  leg["sub"] = new TLegend(0.22, 0.65-0.04, 0.50, 0.65);
  leg["sub"]->AddEntry(hdphi["sub"], "After sub.", "pl");
  for(auto& ll : leg) xjjroot::setleg(ll.second, 0.035);

  xjjroot::setgstyle(1);

  TCanvas* c = new TCanvas("c", "", 600, 600);
  hdphi["raw_scale"]->Draw("pe");
  hdphi["bkg"]->Draw("pe same");
  hdphi["bkg_scale"]->Draw("pe same");
  pa.drawtex(0.23, 0.85, 0.035);
  leg["scale"]->Draw();
  xjjroot::drawCMSleft();
  xjjroot::drawCMSright(Form("%s #sqrt{s_{NN}} = 5.02 TeV", pa.tag("ishi").c_str()));
  output = "plots/" + outsubdir + "_" + pa.tag() + "/chdphi.pdf";
  xjjroot::mkdir(output);
  c->SaveAs(output.c_str());
  delete c;

  c = new TCanvas("c", "", 600, 600);
  hdphi["sub"]->Draw("pe");
  pa.drawtex(0.23, 0.85, 0.035);
  leg["sub"]->Draw();
  xjjroot::drawCMSleft();
  xjjroot::drawCMSright(Form("%s #sqrt{s_{NN}} = 5.02 TeV", pa.tag("ishi").c_str()));
  output = "plots/" + outsubdir + "_" + pa.tag() + "/chdphi_sub.pdf";
  xjjroot::mkdir(output);
  c->SaveAs(output.c_str());

  return 0;
}

void seth(TH1F* h)
{
  xjjroot::sethempty(h, 0, 0.3);
  h->SetMinimum(0);
  h->SetMaximum(h->GetMaximum()*1.4);
  h->GetXaxis()->SetNdivisions(-505);
}

int main(int argc, char* argv[])
{
  if(argc==3)
    return pdana_calchist(argv[1], argv[2]);
  return 1;
}
