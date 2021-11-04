#include <TFile.h>
#include <TH2D.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <string>
#include <map>
#include <vector>

#include "para.h"

#include "xjjrootuti.h"
#include "xjjanauti.h"
#include "xjjcuti.h"
#include "xjjmypdf.h"

std::map<std::string, int> colors = {
  {"HMCMeasured", xjjroot::mycolor_satmiddle["green"]},
  {"HMCTruth", xjjroot::mycolor_satmiddle["azure"]},
  {"HInput", (int)kBlack},
  {"HUnfoldedBayes1", xjjroot::mycolor_satmiddle["cyan"]},
  {"HUnfoldedBayes2", xjjroot::mycolor_satmiddle["orange"]},
  {"HUnfoldedBayes4", xjjroot::mycolor_satmiddle["violet"]},
  {"HUnfoldedBayes10", xjjroot::mycolor_satmiddle["red"]},
  {"HUnfoldedBayes100", xjjroot::mycolor_satmiddle["greenblue"]},
  {"HUnfoldedInvert", xjjroot::mycolor_satmiddle["yellow"]},
};
void DrawThese(std::vector<std::pair<TH1D*, std::string>> tags, xjjroot::mypdf* fpdf);
Djet::param* pa;
int draw(std::string inputname, std::string original, std::string outputdir)
{
  TFile* inforiginal = TFile::Open(original.c_str());
  pa = new Djet::param(inforiginal);
  std::string tvar = "#Deltar";
  if(xjjc::str_contains(inputname, "_dphi.root")) tvar = "#Delta#phi";

  std::map<std::string, TH1D*> h;
  TFile* inf = TFile::Open(inputname.c_str());
  std::vector<std::string> names = {
    "HMCMeasured", "HMCTruth", "HInput", 
    "HUnfoldedBayes1", "HUnfoldedBayes2", "HUnfoldedBayes4", "HUnfoldedBayes10", "HUnfoldedBayes100",
    "HUnfoldedInvert"
  };
  for(auto& t : names)
   {
      std::cout<<t<<std::endl;
      h[t] = (TH1D*)inf->Get(t.c_str());
      h[t]->GetXaxis()->SetTitle(Form("%s bin index", tvar.c_str()));
      h[t]->GetYaxis()->SetTitle(Form("#frac{1}{N_{jet}} #frac{dN}{d%s}", tvar.c_str()));
      xjjroot::setthgrstyle(h[t], colors[t], 20, 0.9, colors[t], 1, 2);
      xjjroot::sethempty(h[t], 0, 0);
    }
  TH2D* HMCResponse = (TH2D*)inf->Get("HMCResponse");
  xjjroot::sethempty(HMCResponse, 0, 0);
  HMCResponse->GetXaxis()->SetNdivisions(-1*xjjana::gethXn(h[names.front()])-100);
  HMCResponse->GetYaxis()->SetNdivisions(-1*xjjana::gethXn(h[names.front()])-100);
  TH2D* HMCResponse_norm = (TH2D*)HMCResponse->Clone("HMCResponse_norm");
  xjjana::normTH2(HMCResponse_norm, "y");
  xjjroot::sethempty(HMCResponse_norm, 0, 0);
  HMCResponse_norm->GetXaxis()->SetNdivisions(-1*xjjana::gethXn(h[names.front()])-100);
  HMCResponse_norm->GetYaxis()->SetNdivisions(-1*xjjana::gethXn(h[names.front()])-100);

  h["HClosure"] = (TH1D*)h["HUnfoldedBayes10"]->Clone("HClosure");
  HClosure->Divide(h["HMCTruth"]);
  xjjroot::setthgrstyle(h[t], kBlack, 20, 0.9, kBlack, 1, 2);
  xjjroot::sethempty(h[t], 0, 0);

  xjjroot::setgstyle(1);
  gStyle->SetPaintTextFormat("1.2f");
  xjjroot::mypdf* fpdf = new xjjroot::mypdf("plots/results/" + outputdir + ".pdf", "c", 600, 600);
  fpdf->getc()->SetGrid();

  fpdf->prepare();
  HMCResponse->Draw("col TEXT");
  xjjroot::drawCMSleft();
  xjjroot::drawCMSright(Form("%s #sqrt{s_{NN}} = 5.02 TeV", pa->tag("ishi").c_str()));
  fpdf->write();

  fpdf->prepare();
  HMCResponse_norm->Draw("col TEXT");
  xjjroot::drawCMSleft();
  xjjroot::drawCMSright(Form("%s #sqrt{s_{NN}} = 5.02 TeV", pa->tag("ishi").c_str()));
  fpdf->write();

  fpdf->getc()->SetGrid(0);

  DrawThese({{h["HMCMeasured"], "MC Reco"},
        {h["HMCTruth"], "MC Gen"}
    }, fpdf);

  DrawThese({{h["HMCMeasured"], "MC Reco"},
        {h["HMCTruth"], "MC Gen"},
          {h["HInput"], "Data"}
    }, fpdf);

  DrawThese({{h["HInput"], "Data"},
        {h["HUnfoldedBayes10"], "Data Unfolded (B-10)"}
    }, fpdf);

  DrawThese({{h["HInput"], "Data"},
        {h["HMCMeasured"], "MC Reco"},
          {h["HMCTruth"], "MC Gen"},
            {h["HUnfoldedBayes10"], "Data Unfolded (B-10)"}
    }, fpdf);

  DrawThese({{h["HMCTruth"], "MC Gen"},
      {h["HUnfoldedBayes1"], "Unfolded (B-1)"},
        {h["HUnfoldedBayes2"], "Unfolded (B-2)"},
          {h["HUnfoldedBayes4"], "Unfolded (B-4)"},
            {h["HUnfoldedBayes10"], "Unfolded (B-10)"},
              {h["HUnfoldedBayes100"], "Unfolded (B-100)"}
    }, fpdf);

  DrawThese({{h["HClosure"], "Unfolded (B-10) / Truth"},
    }, fpdf);

  fpdf->close();
  delete pa;
}

int main(int argc, char* argv[])
{
  if(argc == 4)
    return draw(argv[1], argv[2], argv[3]);
  return 1;
}

void DrawThese(std::vector<std::pair<TH1D*, std::string>> tags, xjjroot::mypdf* fpdf)
{
  // preparation
  double ymin = 1.e+10, yminplus = 1.e+10, ymax = -1.e+10;
  for(auto& t : tags)
    {
      auto thisymin = xjjana::gethminimum(t.first),
        thisymax = xjjana::gethmaximum(t.first);
      ymin = std::min(ymin, thisymin);
      ymax = std::max(ymax, thisymax);
      if(thisymin > 0) yminplus = std::min(yminplus, thisymin);
    }
  TLegend* leg = new TLegend(0.55, 0.85-0.032*1.3*tags.size(), 0.85, 0.85);
  xjjroot::setleg(leg, 0.032);
  xjjroot::setgstyle(1);
  for(auto& t : tags) 
    leg->AddEntry(t.first, t.second.c_str(), "pl");

  // linear
  fpdf->prepare();
  fpdf->getc()->SetLogy(0);
  for(auto& t : tags)
    {
      t.first->SetMinimum(std::min(ymin*1.2, 0.));
      t.first->SetMaximum(ymax*1.5);
    }
  tags.front().first->Draw("AXIS");
  for(auto& t : tags)
    t.first->Draw("ple same");
  leg->Draw();
  pa->drawtex(0.30, 0.85, 0.032, "", 0, -0.032*1.4*4);
  xjjroot::drawCMSleft();
  xjjroot::drawCMSright(Form("%s #sqrt{s_{NN}} = 5.02 TeV", pa->tag("ishi").c_str()));
  fpdf->write();

  // log
  fpdf->prepare();
  fpdf->getc()->SetLogy();
  for(auto& t : tags)
    {
      t.first->SetMinimum(yminplus*0.5);
      t.first->SetMaximum(ymax*5);
    }
  tags.front().first->Draw("AXIS");
  for(auto& t : tags)
    t.first->Draw("ple same");
  leg->Draw();
  pa->drawtex(0.30, 0.85, 0.032, "", 0, -0.032*1.4*4);
  xjjroot::drawCMSleft();
  xjjroot::drawCMSright(Form("%s #sqrt{s_{NN}} = 5.02 TeV", pa->tag("ishi").c_str()));
  fpdf->write();
}
