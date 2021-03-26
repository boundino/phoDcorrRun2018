#include <TFile.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <iostream>
#include <iomanip>

#include "param.h"

#include "xjjcuti.h"
#include "xjjrootuti.h"
#include "pdg.h"

namespace pdana_calchist_
{
  void seth(TH1F* h, std::string ytitle="#frac{1}{N^{#gamma}} #frac{dN^{#gammaD}}{d#phi}", bool forcemaxdigits=true, bool setminmax=true);
  std::string outputdir;
  void makecanvas(TCanvas* c, phoD::param& pa, TLegend* leg, std::string name, std::string comment="");
}

int pdana_calchist(std::string inputname, std::string outsubdir)
{
  TFile* inf = TFile::Open(inputname.c_str());
  phoD::param pa(inf);
  float purity = pa.ishi()?phoD::purity_aa_:phoD::purity_pp_;
  pdana_calchist_::outputdir = outsubdir + "_" + pa.tag();

  std::vector<std::string> types = {"fitweigh", "unweight", "effcorr"};
  std::map<std::string, TH1F*> hdphi;
  for(auto& tt : types)
    {
      hdphi["raw_"+tt] = (TH1F*)inf->Get(Form("hdphi_raw_%s", tt.c_str()));
      hdphi["bkg_"+tt] = (TH1F*)inf->Get(Form("hdphi_bkg_%s", tt.c_str()));
      float nphoton_raw = atof(hdphi["raw_"+tt]->GetTitle());
      float nphoton_bkg = atof(hdphi["bkg_"+tt]->GetTitle());
      hdphi["raw_"+tt]->Scale(1./nphoton_raw);
      hdphi["bkg_"+tt]->Scale(1./nphoton_bkg);

      hdphi["raw_"+tt+"_p"] = (TH1F*)hdphi["raw_"+tt]->Clone(Form("hdphi_raw_%s_p", tt.c_str()));
      hdphi["raw_"+tt+"_p"]->Scale(1./purity);
      hdphi["bkg_"+tt+"_p"] = (TH1F*)hdphi["bkg_"+tt]->Clone(Form("hdphi_bkg_%s_p", tt.c_str()));
      hdphi["bkg_"+tt+"_p"]->Scale((1-purity)/purity);
      hdphi["sub_"+tt] = (TH1F*)hdphi["raw_"+tt+"_p"]->Clone(Form("hdphi_sub_%s", tt.c_str()));
      hdphi["sub_"+tt]->Add(hdphi["bkg_"+tt+"_p"], -1);
      hdphi["sbr_"+tt] = (TH1F*)hdphi["sub_"+tt]->Clone(Form("hdphi_sbr_%s", tt.c_str()));
      hdphi["sbr_"+tt]->Scale(1./pdg::BR_DZERO_KPI);
    }
  hdphi["eff_raw"] = (TH1F*)inf->Get("heff_raw");
  hdphi["eff_bkg"] = (TH1F*)inf->Get("heff_bkg");
  hdphi["dpt_raw"] = (TH1F*)inf->Get("hdpt_raw");
  hdphi["dpt_bkg"] = (TH1F*)inf->Get("hdpt_bkg");
  hdphi["dpt_sig"] = (TH1F*)inf->Get("hdpt_sig");

  std::string output = "rootfiles/" + pdana_calchist_::outputdir + "/calchist.root";
  xjjroot::mkdir(output);
  TFile* outf = new TFile(output.c_str(), "recreate");
  for(auto& hh : hdphi) xjjroot::writehist(hh.second);
  pa.write();
  outf->Close();

  for(auto& hh : hdphi)
    {
      // seth
      if(xjjc::str_contains(hh.first, "eff_"))
        pdana_calchist_::seth(hh.second, "< 1 / #alpha #times #varepsilon >", false);
      else if(xjjc::str_contains(hh.first, "dpt_"))
        pdana_calchist_::seth(hh.second, "", false, false);
      else
        pdana_calchist_::seth(hh.second);
      // setthgrstyle
      if(xjjc::str_contains(hh.first, "bkg"))
        xjjroot::setthgrstyle(hh.second, kBlack, 24, 1, kBlack, 1, 2);
      else if(xjjc::str_contains(hh.first, "sig"))
        xjjroot::setthgrstyle(hh.second, kAzure, 20, 1, kAzure, 1, 2);
      else
        xjjroot::setthgrstyle(hh.second, kBlack, 20, 1, kBlack, 1, 2);
    }
  hdphi["dpt_raw"]->SetMinimum(std::max(hdphi["dpt_sig"]->GetMinimum(), (double)1.)*0.1);
  hdphi["dpt_raw"]->SetMaximum(hdphi["dpt_raw"]->GetMaximum()*5);

  std::map<std::string, TLegend*> leg;
  leg["scale"] = new TLegend(0.22, 0.65-0.04*5, 0.50, 0.65);
  leg["scale"]->SetHeader(Form("p = %.3f", purity), "L");
  leg["scale"]->AddEntry(hdphi["raw_fitweigh_p"], "#frac{1}{p} #times Raw", "pl");
  leg["scale"]->AddEntry(hdphi["bkg_fitweigh_p"], "#frac{1-p}{p} #times Bkg", "pl");
  leg["sub"] = new TLegend(0.22, 0.65-0.04, 0.50, 0.65);
  leg["sub"]->AddEntry(hdphi["sub_fitweigh"], "After sub.", "pl");
  leg["sbr"] = new TLegend(0.22, 0.65-0.04, 0.50, 0.65);
  leg["sbr"]->AddEntry(hdphi["sbr_fitweigh"], "After sub. / BR", "pl");
  leg["raw"] = new TLegend(0.22, 0.65-0.04, 0.50, 0.65);
  leg["raw"]->AddEntry(hdphi["raw_fitweigh"], "Raw", "pl");
  leg["eff"] = new TLegend(0.22, 0.65-0.04*2, 0.50, 0.65);
  leg["eff"]->AddEntry(hdphi["eff_raw"], "Raw", "pl");
  leg["eff"]->AddEntry(hdphi["eff_bkg"], "Bkg", "pl");
  leg["eff_raw"] = new TLegend(0.22, 0.65-0.04, 0.50, 0.65);
  leg["eff_raw"]->AddEntry(hdphi["eff_raw"], "Raw", "pl");
  leg["dpt"] = new TLegend(0.72, 0.75-0.04*3, 0.90, 0.75);
  leg["dpt"]->AddEntry(hdphi["dpt_sig"], "Signal", "pl");
  leg["dpt"]->AddEntry(hdphi["dpt_raw"], "Raw", "pl");
  leg["dpt"]->AddEntry(hdphi["dpt_bkg"], "Bkg", "pl");
  for(auto& ll : leg) xjjroot::setleg(ll.second, 0.035);

  TGaxis::SetExponentOffset(-0.1, 0, "y");
  xjjroot::setgstyle(1);
  TCanvas* c;

  c = new TCanvas("c", "", 600, 600);
  hdphi["eff_raw"]->Draw("pe");
  hdphi["eff_bkg"]->Draw("pe same");
  pdana_calchist_::makecanvas(c, pa, leg["eff"], "cheff", "");

  c = new TCanvas("c", "", 600, 600);
  hdphi["eff_raw"]->Draw("pe");
  pdana_calchist_::makecanvas(c, pa, leg["eff_raw"], "cheff_raw", "");

  c = new TCanvas("c", "", 600, 600);
  c->SetLogy();
  hdphi["dpt_raw"]->Draw("pe");
  hdphi["dpt_bkg"]->Draw("pe same");
  hdphi["dpt_sig"]->Draw("pe same");
  pdana_calchist_::makecanvas(c, pa, leg["dpt"], "chdpt", "");

  std::map<std::string, std::string> tags;
  tags["fitweigh"] = "D eff weight fit";
  tags["unweight"] = "D eff uncorr";
  tags["effcorr"] = "D eff corr";
  for(auto& tt :types)
    {
      c = new TCanvas("c", "", 600, 600);
      hdphi["raw_"+tt]->Draw("pe");
      pdana_calchist_::makecanvas(c, pa, leg["raw"], "chdphi_raw_"+tt, tags[tt]);

      c = new TCanvas("c", "", 600, 600);
      hdphi["raw_"+tt+"_p"]->Draw("pe");
      hdphi["bkg_"+tt+"_p"]->Draw("pe same");
      pdana_calchist_::makecanvas(c, pa, leg["scale"], "chdphi_bkg_"+tt, tags[tt]);

      c = new TCanvas("c", "", 600, 600);
      hdphi["sub_"+tt]->Draw("pe");
      pdana_calchist_::makecanvas(c, pa, leg["sub"], "chdphi_sub_"+tt, tags[tt]);

      c = new TCanvas("c", "", 600, 600);
      hdphi["sbr_"+tt]->Draw("pe");
      pdana_calchist_::makecanvas(c, pa, leg["sbr"], "chdphi_sbr_"+tt, tags[tt]);
    }

  return 0;
}

void pdana_calchist_::seth(TH1F* h, std::string ytitle, bool forcemaxdigits, bool setminmax)
{
  xjjroot::sethempty(h, 0, 0.1);
  if(setminmax)
    {
  h->SetMinimum(0);
  h->SetMaximum(h->GetMaximum()*1.4);
    }
  h->GetXaxis()->SetNdivisions(-505);
  if(forcemaxdigits) h->GetYaxis()->SetMaxDigits(1);
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
