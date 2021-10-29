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

std::map<std::string, TH1D*> h;
std::map<std::string, int> colors = {
  {"HMCReco", xjjroot::mycolor_satmiddle["green"]},
  {"HMCGen", xjjroot::mycolor_satmiddle["azure"]},
  {"HDataReco", (int)kBlack},
  // {"HUnfoldedBayes1", xjjroot::mycolor_satmiddle["cyan"]},
  // {"HUnfoldedBayes2", xjjroot::mycolor_satmiddle["orange"]},
  // {"HUnfoldedBayes4", xjjroot::mycolor_satmiddle["violet"]},
  // {"HUnfoldedBayes10", xjjroot::mycolor_satmiddle["red"]},
  // {"HUnfoldedBayes100", xjjroot::mycolor_satmiddle["greenblue"]},
  // {"HUnfoldedInvert", xjjroot::mycolor_satmiddle["yellow"]},
  {"HMCMatched", xjjroot::mycolor_satmiddle["red"]},
  {"HMCMatchedRecoptGenphi", xjjroot::mycolor_satmiddle["orange"]},
  {"HMCMatchedGenptRecophi", xjjroot::mycolor_satmiddle["violet"]},
  {"HMCMatchedGenptGenphi", xjjroot::mycolor_satmiddle["cyan"]},
};
void DrawThese(std::vector<std::pair<std::string, std::string>> tags, xjjroot::mypdf* f);
Djet::param* pa;
int draw(std::string inputname, std::string outputdir)
{
  TFile* inf = TFile::Open(inputname.c_str());
  pa = new Djet::param(inf, "");
  std::string tvar = "#Deltar";
  if(xjjc::str_contains(inputname, "_dphi.root")) tvar = "#Delta#phi";
  std::vector<std::string> names = {
    "HMCReco", "HMCGen", "HDataReco", 
    "HMCMatched", "HMCMatchedRecoptGenphi", "HMCMatchedGenptRecophi", "HMCMatchedGenptGenphi",
    // "HUnfoldedBayes1", "HUnfoldedBayes2", "HUnfoldedBayes4", "HUnfoldedBayes10", "HUnfoldedBayes100",
    // "HUnfoldedInvert"
  };
  for(auto& t : names)
    {
      h[t] = (TH1D*)inf->Get(t.c_str());
      h[t]->GetXaxis()->SetTitle(Form("%s bin index", tvar.c_str()));
      h[t]->GetYaxis()->SetTitle(Form("#frac{1}{N_{jet}} #frac{dN}{d%s}", tvar.c_str()));
      xjjroot::sethempty(h[t], 0, 0);
    }
  gStyle->SetPaintTextFormat("1.2f");
  TH2D* HResponse = (TH2D*)inf->Get("HResponse");
  xjjana::normTH2(HResponse, "y");
  HResponse->GetXaxis()->SetNdivisions(-1*xjjana::gethXn(h[names.front()])-100);
  HResponse->GetYaxis()->SetNdivisions(-1*xjjana::gethXn(h[names.front()])-100);
  xjjroot::sethempty(HResponse, 0, -0.2);

  xjjroot::setgstyle(3);
  gStyle->SetHatchesLineWidth(2);
  xjjroot::mypdf* fpdf = new xjjroot::mypdf("plots/" + outputdir + "/cunfold_Input.pdf", "c", 700, 600);

  DrawThese({{"HMCMatched", "Matched (Reco p_{T}, Reco #phi,#eta)"},
        {"HMCMatchedRecoptGenphi", "Matched (Reco p_{T}, Gen #phi,#eta)"},
          {"HMCMatchedGenptRecophi", "Matched (Gen p_{T}, Reco #phi,#eta)"},
            {"HMCMatchedGenptGenphi", "Matched (Gen p_{T}, Gen #phi,#eta)"},
              }, fpdf);

  DrawThese({
      {"HMCGen", "MC Gen"},
        {"HMCReco", "MC Reco"},
          }, fpdf);

  DrawThese({
      {"HMCGen", "MC Gen"},
        {"HMCMatchedGenptGenphi", "Matched (Gen p_{T}, Gen #phi,#eta)"},
          {"HMCMatchedGenptRecophi", "Matched (Gen p_{T}, Reco #phi,#eta)"},
            {"HMCMatchedRecoptGenphi", "Matched (Reco p_{T}, Gen #phi,#eta)"},
              {"HMCMatched", "Matched (Reco p_{T}, Reco #phi,#eta)"},
                {"HMCReco", "MC Reco"},
                  }, fpdf);

  DrawThese({{"HMCReco", "MC Reco"},
        {"HMCGen", "MC Gen"},
          {"HDataReco", "Data"}}, fpdf);

  fpdf->prepare();
  // gStyle->SetPadRightMargin(xjjroot::margin_pad_right*3);
  fpdf->getc()->SetLogy(0);
  fpdf->getc()->SetGrid();
  HResponse->Draw("col TEXT");
  xjjroot::drawCMSleft();
  xjjroot::drawCMSright(Form("%s #sqrt{s_{NN}} = 5.02 TeV", pa->tag("ishi").c_str()));
  fpdf->write();

  fpdf->close();

  // DrawThese({{"HDataReco", "Data"},
  //       {"HUnfoldedBayes10", "Data Unfolded (B-10)"}}, outputname+"cunfold_HDataReco-HUnfoldedBayes10");

  // DrawThese({{"HDataReco", "Data"},
  //       {"HMCReco", "MC Reco"},
  //         {"HMCGen", "MC Gen"},
  //           {"HUnfoldedBayes10", "Data Unfolded (B-10)"}}, outputname+"cunfold_All");

  // DrawThese({{"HUnfoldedBayes1", "Unfolded (B-1)"},
  //       {"HUnfoldedBayes2", "Unfolded (B-2)"},
  //         {"HUnfoldedBayes4", "Unfolded (B-4)"},
  //           {"HUnfoldedBayes10", "Unfolded (B-10)"},
  //             {"HUnfoldedBayes100", "Unfolded (B-100)"}}, outputname+"cunfold_Iteration");

  delete pa;
}

int main(int argc, char* argv[])
{
  if(argc == 3)
    return draw(argv[1], argv[2]);
  return 1;
}

void DrawThese(std::vector<std::pair<std::string, std::string>> tags, xjjroot::mypdf* f)
{
  double ymin = 1.e+10, ymax = -1.e+10;
  for(auto& t : tags)
    {
      auto thisymin = xjjana::gethminimum(h[t.first]),
        thisymax = xjjana::gethmaximum(h[t.first]);
      ymin = std::min(ymin, thisymin);
      ymax = std::max(ymax, thisymax);
    }
  TLegend* leg = new TLegend(0.52, 0.85-0.04*tags.size(), 0.75, 0.85);
  xjjroot::setleg(leg, 0.035);
  auto c = f->getc();

  f->prepare();
  c->SetLogy(0);
  int count = 0;
  for(auto& t : tags)
    {
      h[t.first]->SetMinimum(0);
      h[t.first]->SetMaximum(ymax*1.5);
      xjjroot::setthgrstyle(h[t.first], colors[t.first], 20, 0.8, colors[t.first], 1, 2);
      // xjjroot::setthgrstyle(h[t.first], colors[t.first], 20, 0.9, colors[t.first], 1, 2, colors[t.first], 0.2, 1001);
      // xjjroot::setthgrstyle(h[t.first], colors[t.first], 20, 0.9, 0, 0, 0, colors[t.first], 0.3, 3351+count);
      // xjjroot::setthgrstyle(h[t.first], colors[t.first], 20, 0.9, colors[t.first], 1, 2, colors[t.first], 0.5, 3351+count);
      leg->AddEntry(h[t.first], t.second.c_str(), "pl");
      h[t.first]->Draw((count?"ple same":"ple"));
      count++;
    }
  leg->Draw();
  pa->drawtex(0.27, 0.85, 0.035, "cent");
  xjjroot::drawCMSleft();
  xjjroot::drawCMSright(Form("%s #sqrt{s_{NN}} = 5.02 TeV", pa->tag("ishi").c_str()));
  f->write();

  f->prepare();
  c->SetLogy();
  count = 0;
  for(auto& t : tags)
    {
      h[t.first]->SetMinimum(std::max(ymin/2., 1.e-3));
      h[t.first]->SetMaximum(ymax*10);
      h[t.first]->Draw((count?"ple same":"ple"));
      count++;
    }
  leg->Draw();
  pa->drawtex(0.27, 0.85, 0.035, "cent");
  xjjroot::drawCMSleft();
  xjjroot::drawCMSright(Form("%s #sqrt{s_{NN}} = 5.02 TeV", pa->tag("ishi").c_str()));
  f->write();
}
