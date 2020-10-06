#include <TFile.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <iostream>
#include <iomanip>

#include "param.h"

#include "xjjcuti.h"
#include "xjjrootuti.h"

void seth(TH1F* h);
int comphist(std::string inputname, std::string elements, std::string outsubdir, std::string tag, std::string opt)
{
  // prel_finebin_pp_phopt40_pt*_y1.2
  inputname = xjjc::str_replaceall(inputname, "'", "");
  std::vector<std::string> eles = xjjc::str_divide(elements, ",");
  if(eles.empty()) return 2;
  std::map<std::string, TH1F*> hdphi;
  std::map<std::string, std::string> tleg;
  phoD::param* pa;
  for(auto& e : eles)
    {
      std::string input = xjjc::str_replaceall(inputname, "*", e);
      TFile* inf = TFile::Open(input.c_str());
      pa = new phoD::param(inf);
      hdphi[e] = (TH1F*)inf->Get("hdphi_raw");
      hdphi[e]->SetName(Form("hdphi_raw_%s", e.c_str()));
      tleg[e] = pa->tag(tag);
    }
  TLegend* leg = new TLegend(0.22, 0.65-0.035*eles.size(), 0.50, 0.65);
  xjjroot::setleg(leg, 0.03);
  int k=0;
  for(auto& e : eles)
    {
      seth(hdphi[e]);
      xjjroot::setthgrstyle(hdphi[e], xjjroot::mycolor_middle[xjjroot::cc[k]], 20, 1, xjjroot::mycolor_middle[xjjroot::cc[k]], 1, 2, xjjroot::mycolor_light[xjjroot::cc[k]]);
      leg->AddEntry(hdphi[e], tleg[e].c_str(), "pl");
      k++;
    }

  xjjroot::setgstyle(1);
  TCanvas* c = new TCanvas("c", "", 600, 600);
  hdphi[eles[0]]->Draw(opt.c_str());
  for(auto& e : eles) hdphi[e]->Draw(Form("%s same", opt.c_str()));
  for(auto& e : eles) hdphi[e]->Draw("pe same");
  pa->drawtex(0.23, 0.85, 0.035, tag);
  leg->Draw();
  xjjroot::drawCMSleft();
  xjjroot::drawCMSright(Form("%s #sqrt{s_{NN}} = 5.02 TeV", pa->tag("ishi").c_str()));
  std::string output = "plots/compare/" + outsubdir + "_" + tag + "/chdphi_raw.pdf";
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
  if(argc==6)
    return comphist(argv[1], argv[2], argv[3], argv[4], argv[5]);
  return 1;
}
