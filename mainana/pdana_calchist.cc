#include <TFile.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <iostream>
#include <iomanip>

#include "param.h"

#include "xjjcuti.h"
#include "xjjrootuti.h"

namespace pdana_calchist_
{
  void seth(TH1F* h, std::string ytitle="#frac{1}{N^{#gamma}} #frac{dN^{#gammaD}}{d#phi}");
  std::string outputdir;
  void makecanvas(TCanvas* c, phoD::param& pa, TLegend* leg, std::string name, std::string comment="");
}

int pdana_calchist(std::string inputname, std::string outsubdir)
{
  TFile* inf = TFile::Open(inputname.c_str());
  phoD::param pa(inf);
  float purity = pa.ishi()?phoD::purity_aa_:phoD::purity_pp_;
  pdana_calchist_::outputdir = outsubdir + "_" + pa.tag();

  std::map<std::string, TH1F*> hdphi;
  hdphi["raw"] = (TH1F*)inf->Get("hdphi_raw");
  hdphi["bkg"] = (TH1F*)inf->Get("hdphi_bkg");
  hdphi["raw_unweight"] = (TH1F*)inf->Get("hdphi_raw_unweight");
  hdphi["bkg_unweight"] = (TH1F*)inf->Get("hdphi_bkg_unweight");
  float nphoton_raw = atof(hdphi["raw"]->GetTitle());
  float nphoton_bkg = atof(hdphi["bkg"]->GetTitle());
  hdphi["raw"]->Scale(1./nphoton_raw);
  hdphi["bkg"]->Scale(1./nphoton_bkg);
  hdphi["raw_unweight"]->Scale(1./nphoton_raw);
  hdphi["bkg_unweight"]->Scale(1./nphoton_bkg);

  hdphi["raw_p"] = (TH1F*)hdphi["raw"]->Clone("hdphi_raw_p");
  hdphi["raw_p"]->Scale(1./purity);
  hdphi["bkg_p"] = (TH1F*)hdphi["bkg"]->Clone("hdphi_bkg_p");
  hdphi["bkg_p"]->Scale((1-purity)/purity);
  hdphi["sub"] = (TH1F*)hdphi["raw_p"]->Clone("hdphi_sub");
  hdphi["sub"]->Add(hdphi["bkg_p"], -1);

  hdphi["raw_unweight_p"] = (TH1F*)hdphi["raw_unweight"]->Clone("hdphi_raw_unweight_p");
  hdphi["raw_unweight_p"]->Scale(1./purity);
  hdphi["bkg_unweight_p"] = (TH1F*)hdphi["bkg_unweight"]->Clone("hdphi_bkg_unweight_p");
  hdphi["bkg_unweight_p"]->Scale((1-purity)/purity);
  hdphi["sub_unweight"] = (TH1F*)hdphi["raw_unweight_p"]->Clone("hdphi_sub_unweight");
  hdphi["sub_unweight"]->Add(hdphi["bkg_unweight_p"], -1);

  std::string output = "rootfiles/" + pdana_calchist_::outputdir + "/calchist.root";
  xjjroot::mkdir(output);
  TFile* outf = new TFile(output.c_str(), "recreate");
  for(auto& hh : hdphi) xjjroot::writehist(hh.second);
  pa.write();
  outf->Close();

  for(auto& hh : hdphi) pdana_calchist_::seth(hh.second);
  xjjroot::setthgrstyle(hdphi["raw"], kBlack, 20, 1, kBlack, 1, 2);
  xjjroot::setthgrstyle(hdphi["raw_unweight"], kBlack, 20, 1, kBlack, 1, 2);
  xjjroot::setthgrstyle(hdphi["bkg"], kGray, 24, 1, kGray, 1, 2);
  xjjroot::setthgrstyle(hdphi["bkg_unweight"], kGray, 24, 1, kGray, 1, 2);
  xjjroot::setthgrstyle(hdphi["raw_p"], kBlack, 20, 1, kBlack, 1, 2);
  xjjroot::setthgrstyle(hdphi["raw_unweight_p"], kBlack, 20, 1, kBlack, 1, 2);
  xjjroot::setthgrstyle(hdphi["bkg_p"], kBlack, 24, 1, kBlack, 1, 2);
  xjjroot::setthgrstyle(hdphi["bkg_unweight_p"], kBlack, 24, 1, kBlack, 1, 2);
  xjjroot::setthgrstyle(hdphi["sub"], kBlack, 20, 1, kBlack, 1, 2);
  xjjroot::setthgrstyle(hdphi["sub_unweight"], kBlack, 20, 1, kBlack, 1, 2);

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
  pdana_calchist_::makecanvas(c, pa, leg["raw"], "chdphi_raw", "D eff corr");

  c = new TCanvas("c", "", 600, 600);
  hdphi["raw_unweight"]->Draw("pe");
  pdana_calchist_::makecanvas(c, pa, leg["raw"], "chdphi_raw_unweight", "D eff uncorr");

  c = new TCanvas("c", "", 600, 600);
  hdphi["raw_p"]->Draw("pe");
  hdphi["bkg_p"]->Draw("pe same");
  pdana_calchist_::makecanvas(c, pa, leg["scale"], "chdphi", "D eff corr");

  c = new TCanvas("c", "", 600, 600);
  hdphi["raw_unweight_p"]->Draw("pe");
  hdphi["bkg_unweight_p"]->Draw("pe same");
  pdana_calchist_::makecanvas(c, pa, leg["scale"], "chdphi_unweight", "D eff uncorr");

  c = new TCanvas("c", "", 600, 600);
  hdphi["sub"]->Draw("pe");
  pdana_calchist_::makecanvas(c, pa, leg["sub"], "chdphi_sub", "D eff corr");

  c = new TCanvas("c", "", 600, 600);
  hdphi["sub_unweight"]->Draw("pe");
  pdana_calchist_::makecanvas(c, pa, leg["sub"], "chdphi_sub_unweight", "D eff uncorr");

  return 0;
}

void pdana_calchist_::seth(TH1F* h, std::string ytitle)
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

void pdana_calchist_::makecanvas(TCanvas* c, phoD::param& pa, TLegend* leg, std::string name, std::string comment)
{
  c->cd();
  pa.drawtex(0.23, 0.85, 0.035);
  leg->Draw();
  xjjroot::drawCMSleft();
  xjjroot::drawCMSright(Form("%s #sqrt{s_{NN}} = 5.02 TeV", pa.tag("ishi").c_str()));
  xjjroot::drawtex(0.92, 0.80, comment.c_str(), 0.035, 33, 62);
  xjjroot::drawcomment(pdana_calchist_::outputdir, "lb");
  std::string output = "plots/" + pdana_calchist_::outputdir + "/" + name + ".pdf";
  xjjroot::mkdir(output);
  c->SaveAs(output.c_str());
  delete c;
}
