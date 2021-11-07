#include <TFile.h>
#include <TH2D.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TKey.h>
#include <TObjString.h>
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
  {"HUnfoldedSVD1", xjjroot::mycolor_light["cyan"]},
  {"HUnfoldedSVD2", xjjroot::mycolor_light["orange"]},
  {"HUnfoldedSVD4", xjjroot::mycolor_light["violet"]},
  {"HUnfoldedInvert", xjjroot::mycolor_satmiddle["yellow"]},
};
void DrawThese(std::vector<std::pair<TH1D*, std::string>> tags, xjjroot::mypdf* fpdf, float forceymin=0, float forceymax=-1);
Djet::param* pa;
int draw(std::string inputname, std::string original, std::string outputdir)
{
  TFile* inforiginal = TFile::Open(original.c_str());
  pa = new Djet::param(inforiginal);
  std::string var = "dr";
  if(xjjc::str_contains(inputname, "_dphi.root")) var = "dphi";

  std::map<std::string, TH1D*> h;
  TFile* inf = TFile::Open(inputname.c_str());
  std::vector<std::string> names = {
    "HMCMeasured", "HMCTruth", "HInput", 
    "HUnfoldedBayes", "HUnfoldedInvert", "HUnfoldedSVD"
  };
  TKey* key; TIter nextkey(inf->GetListOfKeys());
  std::vector<float> Iter_Bayes, Iter_SVD; 
  while((key = (TKey*)nextkey()))
    {
      std::string classname(key->GetClassName()), keyname(key->GetName());
      if(classname != "TH1D") continue;
      if(xjjc::str_contains(keyname, "HUnfoldedBayes")) 
        Iter_Bayes.push_back(atof(xjjc::str_eraseall(keyname, "HUnfoldedBayes").c_str()));
      if(xjjc::str_contains(keyname, "HUnfoldedSVD")) 
        Iter_SVD.push_back(atof(xjjc::str_eraseall(keyname, "HUnfoldedSVD").c_str()));
      h[keyname] = (TH1D*)inf->Get(keyname.c_str());
      h[keyname]->GetXaxis()->SetTitle(Form("%s bin index", Djet::vartex[var].c_str()));
      h[keyname]->GetYaxis()->SetTitle("a.u.");
      h[keyname]->Scale(1./h[keyname]->Integral());
      int cc = (colors.find(keyname) != colors.end())?colors[keyname]:kBlack;
      xjjroot::setthgrstyle(h[keyname], cc, 20, 0.9, cc, 1, 2);
      xjjroot::sethempty(h[keyname], 0, 0);
    }
  int nbin = h["HInput"]->GetXaxis()->GetNbins();
  std::vector<TGraph*> grBayes(nbin), grSVD(nbin);
  for(int i=0; i<nbin; i++)
    {
      std::vector<float> vbinBayes, vbinSVD;
      int cc = kBlack;
      if((var=="dr" && i>4) || (var=="dphi" && (i>1 && i<8))) cc = kGray;
      for(auto& j : Iter_Bayes)
        vbinBayes.push_back(h[Form("HUnfoldedBayes%.0f", j)]->GetBinContent(i+1));
      grBayes[i] = new TGraph(Iter_Bayes.size(), Iter_Bayes.data(), vbinBayes.data());
      grBayes[i]->SetName(Form("grBayes%d", i));
      xjjana::gScale(grBayes[i], 1./vbinBayes[0]);
      xjjroot::setthgrstyle(grBayes[i], cc, 20, 0.7, cc, 1, 2);

      for(auto& j : Iter_SVD)
        vbinSVD.push_back(h[Form("HUnfoldedSVD%.0f", j)]->GetBinContent(i+1));
      grSVD[i] = new TGraph(Iter_SVD.size(), Iter_SVD.data(), vbinSVD.data());
      grSVD[i]->SetName(Form("grSVD%d", i));
      xjjana::gScale(grSVD[i], 1./vbinSVD[0]);
      xjjroot::setthgrstyle(grSVD[i], cc, 20, 0.7, cc, 1, 2);
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

  h["HClosure_before"] = (TH1D*)h["HInput"]->Clone("HClosure_before");
  h["HClosure_before"]->Divide(h["HMCTruth"]);
  xjjroot::setthgrstyle(h["HClosure_before"], kGray, 20, 0.9, kGray, 1, 2);
  xjjroot::sethempty(h["HClosure_before"], 0, 0);
  h["HClosure_before"]->GetYaxis()->SetTitle("Ratio");

  h["HClosure"] = (TH1D*)h["HUnfoldedBayes10"]->Clone("HClosure");
  h["HClosure"]->Divide(h["HMCTruth"]);
  xjjroot::setthgrstyle(h["HClosure"], kBlack, 20, 0.9, kBlack, 1, 2);
  xjjroot::sethempty(h["HClosure"], 0, 0);
  h["HClosure"]->GetYaxis()->SetTitle("Ratio");

  TH2D* hempty_Bayes = new TH2D("hempty_Bayes", ";Iterations;", 10, 0.6, 300, 10, 0, 2);
  xjjroot::sethempty(hempty_Bayes);
  TH2D* hempty_SVD = new TH2D("hempty_SVD", ";Iterations;", 10, 0, 10, 10, 0, 2);
  xjjroot::sethempty(hempty_SVD);

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

  fpdf->prepare();
  fpdf->getc()->SetLogy(0);
  fpdf->getc()->SetLogx();
  hempty_Bayes->Draw("AXIS");
  for(auto& gr : grBayes) gr->Draw("pl same");
  xjjroot::drawCMSleft();
  xjjroot::drawCMSright(Form("%s #sqrt{s_{NN}} = 5.02 TeV", pa->tag("ishi").c_str()));
  xjjroot::drawcomment("Bayes");
  fpdf->write();

  fpdf->prepare();
  fpdf->getc()->SetLogx(0);
  hempty_SVD->Draw("AXIS");
  for(auto& gr : grSVD) gr->Draw("pl same");
  xjjroot::drawCMSleft();
  xjjroot::drawCMSright(Form("%s #sqrt{s_{NN}} = 5.02 TeV", pa->tag("ishi").c_str()));
  xjjroot::drawcomment("SVD");
  fpdf->write();

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

  DrawThese({{h["HMCTruth"], "MC Gen"},
        {h["HUnfoldedBayes10"], "Unfolded (B-10)"},
          {h["HUnfoldedSVD1"], "Unfolded (SVD-1)"},
            {h["HUnfoldedSVD4"], "Unfolded (SVD-4)"},
              {h["HUnfoldedInvert"], "Unfolded (Invert)"},
                }, fpdf);

  DrawThese({{h["HClosure_before"], "Measured / Truth"},
        {h["HClosure"], "Unfolded (B-10) / Truth"}
    }, fpdf, 0.6, 1.6);

  fpdf->close();
  delete pa;
}

int main(int argc, char* argv[])
{
  if(argc == 4)
    return draw(argv[1], argv[2], argv[3]);
  return 1;
}

void DrawThese(std::vector<std::pair<TH1D*, std::string>> tags, xjjroot::mypdf* fpdf, float forceymin, float forceymax)
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
      if(forceymin==0)
        t.first->SetMinimum(std::min(ymin*1.2, 0.));
      else
        t.first->SetMinimum(forceymin);
      if(forceymax<0)
        t.first->SetMaximum(ymax*1.5);
      else
        t.first->SetMaximum(forceymax);
    }
  tags.front().first->Draw("AXIS");
  for(auto& t : tags)
    t.first->Draw("pe same");
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
    t.first->Draw("pe same");
  leg->Draw();
  pa->drawtex(0.30, 0.85, 0.032, "", 0, -0.032*1.4*4);
  xjjroot::drawCMSleft();
  xjjroot::drawCMSright(Form("%s #sqrt{s_{NN}} = 5.02 TeV", pa->tag("ishi").c_str()));
  fpdf->write();
}
