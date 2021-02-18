#include <TFile.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <iostream>
#include <iomanip>

#include "param.h"

#include "xjjcuti.h"
#include "xjjrootuti.h"

void seth(TH1F* h, std::string ytitle="#frac{1}{N^{#gamma}} #frac{dN^{#gammaD}}{d#phi}");
int pdana_calchist(std::string inputname, std::string outsubdir)
{
  TFile* inf = TFile::Open(inputname.c_str());
  phoD::param pa(inf);
  float purity = pa.ishi()?phoD::purity_aa_:phoD::purity_pp_;

  std::map<std::string, TH1F*> hdphi;
  hdphi["raw"] = (TH1F*)inf->Get("hdphi_raw");
  hdphi["bkg"] = (TH1F*)inf->Get("hdphi_bkg");
  float nphoton_raw = atof(hdphi["raw"]->GetTitle());
  float nphoton_bkg = atof(hdphi["bkg"]->GetTitle());
  hdphi["raw"]->Scale(1./nphoton_raw);
  hdphi["bkg"]->Scale(1./nphoton_bkg);

  hdphi["raw_p"] = (TH1F*)hdphi["raw"]->Clone("hdphi_raw_p");
  hdphi["raw_p"]->Scale(1./purity);
  hdphi["bkg_p"] = (TH1F*)hdphi["bkg"]->Clone("hdphi_bkg_p");
  hdphi["bkg_p"]->Scale((1-purity)/purity);
  hdphi["sub"] = (TH1F*)hdphi["raw_p"]->Clone("hdphi_sub");
  hdphi["sub"]->Add(hdphi["bkg_p"], -1);

  std::string output = "rootfiles/" + outsubdir + "_" + pa.tag() + "/calchist.root";
  xjjroot::mkdir(output);
  TFile* outf = new TFile(output.c_str(), "recreate");
  for(auto& hh : hdphi) xjjroot::writehist(hh.second);
  pa.write();
  outf->Close();

  for(auto& hh : hdphi) seth(hh.second);
  xjjroot::setthgrstyle(hdphi["raw"], kBlack, 20, 1, kBlack, 1, 2);
  xjjroot::setthgrstyle(hdphi["bkg"], kGray, 24, 1, kGray, 1, 2);
  xjjroot::setthgrstyle(hdphi["raw_p"], kBlack, 20, 1, kBlack, 1, 2);
  xjjroot::setthgrstyle(hdphi["bkg_p"], kBlack, 24, 1, kBlack, 1, 2);
  xjjroot::setthgrstyle(hdphi["sub"], kBlack, 20, 1, kBlack, 1, 2);

  std::map<std::string, TLegend*> leg;
  leg["scale"] = new TLegend(0.22, 0.65-0.04*5, 0.50, 0.65);
  leg["scale"]->SetHeader(Form("p = %.3f", purity), "L");
  leg["scale"]->AddEntry(hdphi["raw_p"], "#frac{1}{p} #times Raw", "pl");
  // leg["scale"]->AddEntry(hdphi["bkg"], "Bkg (sideband)", "pl");
  leg["scale"]->AddEntry(hdphi["bkg_p"], "#frac{1-p}{p} #times Bkg", "pl");
  leg["sub"] = new TLegend(0.22, 0.65-0.04, 0.50, 0.65);
  leg["sub"]->AddEntry(hdphi["sub"], "After sub.", "pl");
  leg["raw"] = new TLegend(0.22, 0.65-0.04, 0.50, 0.65);
  leg["raw"]->AddEntry(hdphi["raw"], "Raw", "pl");
  for(auto& ll : leg) xjjroot::setleg(ll.second, 0.035);

  TGaxis::SetExponentOffset(-0.1, 0, "y");
  xjjroot::setgstyle(1);
  TCanvas* c;

  c = new TCanvas("c", "", 600, 600);
  hdphi["raw"]->Draw("pe");
  pa.drawtex(0.23, 0.85, 0.035);
  leg["raw"]->Draw();
  xjjroot::drawCMSleft();
  xjjroot::drawCMSright(Form("%s #sqrt{s_{NN}} = 5.02 TeV", pa.tag("ishi").c_str()));
  output = "plots/" + outsubdir + "_" + pa.tag() + "/chdphi_raw.pdf";
  xjjroot::mkdir(output);
  c->SaveAs(output.c_str());
  delete c;

  c = new TCanvas("c", "", 600, 600);
  // hdphi["raw"]->Draw("pe");
  // hdphi["bkg"]->Draw("pe same");
  hdphi["raw_p"]->Draw("pe");
  hdphi["bkg_p"]->Draw("pe same");
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

void seth(TH1F* h, std::string ytitle)
{
  xjjroot::sethempty(h, 0, 0.1);
  h->SetMinimum(0);
  h->SetMaximum(h->GetMaximum()*1.4);
  h->GetXaxis()->SetNdivisions(-505);
  h->GetYaxis()->SetMaxDigits(1);
  h->GetYaxis()->SetTitle(ytitle.c_str());
}

int main(int argc, char* argv[])
{
  if(argc==3)
    return pdana_calchist(argv[1], argv[2]);
  return 1;
}
