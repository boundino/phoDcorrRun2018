#include <TFile.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <iostream>
#include <iomanip>

#include "param.h"

#include "xjjcuti.h"
#include "xjjrootuti.h"
#include "pdg.h"

namespace djana_calchist_
{
  void seth(TH1F* h, std::string ytitle="#frac{1}{N^{jet}} #frac{dN^{jD}}{d#phi}", bool forcemaxdigits=true, bool setminmax=true);
  std::string outputdir;
  void makecanvas(TCanvas* c, Djet::param& pa, TLegend* leg, std::string name, std::string comment="");
}

int djana_calchist(std::string inputname, std::string outsubdir)
{
  TFile* inf = TFile::Open(inputname.c_str());
  Djet::param pa(inf);
  djana_calchist_::outputdir = outsubdir + "_" + pa.tag();

  std::vector<std::string> types = {"fitweigh", "unweight", "effcorr"};
  std::map<std::string, TH1F*> hdphi;
  for(auto& tt : types)
    {
      hdphi["raw_"+tt] = (TH1F*)inf->Get(Form("hdphi_raw_%s", tt.c_str()));
      float njet_raw = atof(hdphi["raw_"+tt]->GetTitle());
      hdphi["raw_"+tt]->Scale(1./njet_raw);

      hdphi["sbr_"+tt] = (TH1F*)hdphi["raw_"+tt]->Clone(Form("hdphi_sbr_%s", tt.c_str()));
      hdphi["sbr_"+tt]->Scale(1./pdg::BR_DZERO_KPI);
    }
  hdphi["eff_raw"] = (TH1F*)inf->Get("heff_raw");
  hdphi["dpt_raw"] = (TH1F*)inf->Get("hdpt_raw");
  hdphi["dpt_bkg"] = (TH1F*)inf->Get("hdpt_bkg");
  hdphi["dpt_sig"] = (TH1F*)inf->Get("hdpt_sig");

  std::string output = "rootfiles/" + djana_calchist_::outputdir + "/calchist.root";
  xjjroot::mkdir(output);
  TFile* outf = new TFile(output.c_str(), "recreate");
  for(auto& hh : hdphi) xjjroot::writehist(hh.second);
  pa.write();
  outf->Close();

  for(auto& hh : hdphi)
    {
      // seth
      if(xjjc::str_contains(hh.first, "eff_"))
        djana_calchist_::seth(hh.second, "< 1 / #alpha #times #varepsilon >", false);
      else if(xjjc::str_contains(hh.first, "dpt_"))
        djana_calchist_::seth(hh.second, "", false, false);
      else
        djana_calchist_::seth(hh.second);
      // setthgrstyle
      if(xjjc::str_contains(hh.first, "bkg"))
        xjjroot::setthgrstyle(hh.second, kBlack, 24, 1, kBlack, 7, 2);
      else if(xjjc::str_contains(hh.first, "sig"))
        xjjroot::setthgrstyle(hh.second, kAzure, 20, 1, kAzure, 1, 2);
      else
        xjjroot::setthgrstyle(hh.second, kBlack, 20, 1, kBlack, 1, 2);
    }
  hdphi["dpt_raw"]->SetMinimum(hdphi["dpt_sig"]->GetMinimum()*0.1);
  hdphi["dpt_raw"]->SetMaximum(hdphi["dpt_raw"]->GetMaximum()*5);

  std::map<std::string, TLegend*> leg;
  leg["sbr"] = new TLegend(0.22, 0.65-0.04, 0.50, 0.65);
  leg["sbr"]->AddEntry(hdphi["sbr_fitweigh"], "After sub. / BR", "pl");
  leg["raw"] = new TLegend(0.22, 0.65-0.04, 0.50, 0.65);
  leg["raw"]->AddEntry(hdphi["raw_fitweigh"], "Raw", "pl");
  leg["eff"] = new TLegend(0.22, 0.65-0.04, 0.50, 0.65);
  leg["eff"]->AddEntry(hdphi["eff_raw"], "Raw", "pl");
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
  djana_calchist_::makecanvas(c, pa, leg["eff"], "cheff", "");

  c = new TCanvas("c", "", 600, 600);
  c->SetLogy();
  hdphi["dpt_raw"]->Draw("pe");
  hdphi["dpt_bkg"]->Draw("pe same");
  hdphi["dpt_sig"]->Draw("pe same");
  djana_calchist_::makecanvas(c, pa, leg["dpt"], "chdpt", "");

  std::map<std::string, std::string> tags;
  tags["fitweigh"] = "D eff weight fit";
  tags["unweight"] = "D eff uncorr";
  tags["effcorr"] = "D eff corr";
  for(auto& tt :types)
    {
      c = new TCanvas("c", "", 600, 600);
      hdphi["raw_"+tt]->Draw("pe");
      djana_calchist_::makecanvas(c, pa, leg["raw"], "chdphi_raw_"+tt, tags[tt]);

      c = new TCanvas("c", "", 600, 600);
      hdphi["sbr_"+tt]->Draw("pe");
      djana_calchist_::makecanvas(c, pa, leg["sbr"], "chdphi_sbr_"+tt, tags[tt]);
    }

  return 0;
}

void djana_calchist_::seth(TH1F* h, std::string ytitle, bool forcemaxdigits, bool setminmax)
{
  xjjroot::sethempty(h, 0, 0.1);
  if(setminmax)
    {
      h->SetMinimum(0);
      h->SetMaximum(h->GetMaximum()*1.7);
    }
  h->GetXaxis()->SetNdivisions(-505);
  if(forcemaxdigits) h->GetYaxis()->SetMaxDigits(1);
  h->GetYaxis()->SetTitle(ytitle.c_str());
}

int main(int argc, char* argv[])
{
  if(argc==3)
    return djana_calchist(argv[1], argv[2]);
  return 1;
}

void djana_calchist_::makecanvas(TCanvas* c, Djet::param& pa, TLegend* leg, std::string name, std::string comment)
{
  c->cd();
  pa.drawtex(0.23, 0.85, 0.035);
  leg->Draw();
  xjjroot::drawCMSleft();
  xjjroot::drawCMSright(Form("%s #sqrt{s_{NN}} = 5.02 TeV", pa.tag("ishi").c_str()));
  xjjroot::drawtex(0.92, 0.80, comment.c_str(), 0.035, 33, 62);
  xjjroot::drawcomment(djana_calchist_::outputdir, "lb");
  std::string output = "plots/" + djana_calchist_::outputdir + "/" + name + ".pdf";
  xjjroot::mkdir(output);
  c->SaveAs(output.c_str());
  delete c;
}
