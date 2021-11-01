#include <TFile.h>
#include <TH2D.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <string>
#include <map>
#include <vector>

#include "para.h"
#include "bins.h"

#include "xjjrootuti.h"
#include "xjjanauti.h"
#include "xjjcuti.h"
#include "xjjmypdf.h"

std::map<std::string, TH1D*> h;
std::map<std::string, int> colors = {
  {"HMCReco", xjjroot::mycolor_satmiddle["green"]},
  {"HMCReco_norm", xjjroot::mycolor_satmiddle["green"]},
  {"HMCGen", xjjroot::mycolor_satmiddle["azure"]},
  {"HMCGen_norm", xjjroot::mycolor_satmiddle["azure"]},
  {"HDataReco", (int)kBlack},
  {"HDataReco_norm", (int)kBlack},
  {"HDataReco_nojercorr", (int)kBlack},
  {"HDataReco_nojercorr_norm", (int)kBlack},
  {"HMCMatched", xjjroot::mycolor_satmiddle["red"]},
  {"HMCMatched_norm", xjjroot::mycolor_satmiddle["red"]},
  {"HMCMatchedRecoptGenphi", xjjroot::mycolor_satmiddle["orange"]},
  {"HMCMatchedRecoptGenphi_norm", xjjroot::mycolor_satmiddle["orange"]},
  {"HMCMatchedGenptRecophi", xjjroot::mycolor_satmiddle["violet"]},
  {"HMCMatchedGenptRecophi_norm", xjjroot::mycolor_satmiddle["violet"]},
  {"HMCMatchedGenptGenphi", xjjroot::mycolor_satmiddle["cyan"]},
  {"HMCMatchedGenptGenphi_norm", xjjroot::mycolor_satmiddle["cyan"]},
  {"hrefpt_match_reco", xjjroot::mycolor_satmiddle["red"]},
  {"hrefpt_match_reco_jD", xjjroot::mycolor_satmiddle["red"]},
  {"hrefpt_match_gen", xjjroot::mycolor_satmiddle["blue"]},
  {"hrefpt_match_gen_jD", xjjroot::mycolor_satmiddle["blue"]},
  {"hratio_GenptRecophi_RecoptRecophi", kGray},
  {"hratio_GenptRecophi_RecoptRecophi_norm", kBlack},
};
void DrawThese(std::vector<std::pair<TH1D*, std::string>> tags, xjjroot::mypdf* f, std::string comment="");
Djet::param* pa;
int draw(std::string inputname, std::string outputdir)
{
  TFile* inf = TFile::Open(inputname.c_str());
  pa = new Djet::param(inf, "");
  std::string var = "dr";
  if(xjjc::str_contains(inputname, "_dphi.root")) var = "dphi";
  xjjana::bins<float> vb(Djet::bins[var]);

  // read histograms
  std::vector<std::string> names = {
    "HMCReco", "HMCGen", "HDataReco", 
    "HMCMatched", "HMCMatchedRecoptGenphi", "HMCMatchedGenptRecophi", "HMCMatchedGenptGenphi",
    "HDataReco_nojercorr", "hratio_GenptRecophi_RecoptRecophi"
  };
  for(auto& t : names)
    {
      std::cout<<t<<std::endl;
      h[t] = (TH1D*)inf->Get(Form("%s-original", t.c_str()));
      h[t+"_norm"] = (TH1D*)inf->Get(Form("%s_norm-original", t.c_str()));
      xjjroot::sethempty(h[t], 0, 0);
      xjjroot::setthgrstyle(h[t], colors[t], 20, 0.8, colors[t], 1, 2);
      xjjroot::sethempty(h[t+"_norm"], 0, 0);
      xjjroot::setthgrstyle(h[t+"_norm"], colors[t+"_norm"], 20, 0.8, colors[t+"_norm"], 1, 2);
    }
  // read jet pT hists
  std::vector<std::string> names_refpt = {"hrefpt_match_reco", "hrefpt_match_gen"};
  for(auto& t : names_refpt)
    {
      h[t] = (TH1D*)inf->Get(t.c_str());
      xjjroot::sethempty(h[t], 0, 0);
      xjjroot::setthgrstyle(h[t], colors[t], 20, 0.8, colors[t], 1, 2);
    }

  // draw
  xjjroot::setgstyle(3);
  gStyle->SetHatchesLineWidth(2);
  xjjroot::mypdf* fpdf = new xjjroot::mypdf("plots/prep/" + outputdir + ".pdf", "c", 700, 600);

  DrawThese({
      {h["HMCGen"], "MC Gen"},
        {h["HMCMatchedGenptGenphi"], "Matched (Gen p_{T}, Gen #phi,#eta)"},
          {h["HMCMatchedGenptRecophi"], "Matched (Gen p_{T}, Reco #phi,#eta)"},
            {h["HMCMatchedRecoptGenphi"], "Matched (Reco p_{T}, Gen #phi,#eta)"},
              {h["HMCMatched"], "Matched (Reco p_{T}, Reco #phi,#eta)"},
                {h["HMCReco"], "MC Reco"},
                  }, fpdf);

  DrawThese({
      {h["HMCGen_norm"], "MC Gen"},
        {h["HMCMatchedGenptGenphi_norm"], "Matched (Gen p_{T}, Gen #phi,#eta)"},
          {h["HMCMatchedGenptRecophi_norm"], "Matched (Gen p_{T}, Reco #phi,#eta)"},
            {h["HMCMatchedRecoptGenphi_norm"], "Matched (Reco p_{T}, Gen #phi,#eta)"},
              {h["HMCMatched_norm"], "Matched (Reco p_{T}, Reco #phi,#eta)"},
                {h["HMCReco_norm"], "MC Reco"},
                  }, fpdf);

  DrawThese({{h["hratio_GenptRecophi_RecoptRecophi"], "N_{jet} normalized"},
        {h["hratio_GenptRecophi_RecoptRecophi_norm"], "N_{jD} normalized"}}, fpdf);

  DrawThese({{h["HMCMatched"], "MC Matched"},
        {h["HMCGen"], "MC Gen"},
          {h["HDataReco_nojercorr"], "Data (No JER corr)"}}, fpdf);

  DrawThese({{h["HMCMatched"], "MC Matched"},
        {h["HMCGen"], "MC Gen"},
          {h["HDataReco"], "Data (Correct JER)"}}, fpdf);

  DrawThese({{h["HMCMatched_norm"], "MC Matched"},
        {h["HMCGen_norm"], "MC Gen"},
          {h["HDataReco_nojercorr_norm"], "Data (No JER corr)"}}, fpdf);

  DrawThese({{h["HMCMatched_norm"], "MC Matched"},
        {h["HMCGen_norm"], "MC Gen"},
          {h["HDataReco_norm"], "Data (Correct JER)"}}, fpdf);

  DrawThese({{h["hrefpt_match_gen"], "Gen p_{T} > 80 GeV"},
        {h["hrefpt_match_reco"], "Reco p_{T} > 80 GeV"}}, fpdf, "Number of jets vs. ref p_{T}");
 
  fpdf->close();

  // DrawThese({{h["HMCMatched"], "Matched (Reco p_{T}, Reco #phi,#eta)"},
  //       {h["HMCMatchedRecoptGenphi"], "Matched (Reco p_{T}, Gen #phi,#eta)"},
  //         {h["HMCMatchedGenptRecophi"], "Matched (Gen p_{T}, Reco #phi,#eta)"},
  //           {h["HMCMatchedGenptGenphi"], "Matched (Gen p_{T}, Gen #phi,#eta)"},
  //             }, fpdf);

  // DrawThese({
  //     {h["HMCGen"], "MC Gen"},
  //       {h["HMCReco"], "MC Reco"},
  //         }, fpdf);

  // DrawThese({{h["HDataReco"], "Data"},
  //       {h["HUnfoldedBayes10"], "Data Unfolded (B-10)"}}, outputname+"cunfold_HDataReco-HUnfoldedBayes10");

  // DrawThese({{h["HDataReco"], "Data"},
  //       {h["HMCReco"], "MC Reco"},
  //         {h["HMCGen"], "MC Gen"},
  //           {h["HUnfoldedBayes10"], "Data Unfolded (B-10)"}}, outputname+"cunfold_All");

  // DrawThese({{h["HUnfoldedBayes1"], "Unfolded (B-1)"},
  //       {h["HUnfoldedBayes2"], "Unfolded (B-2)"},
  //         {h["HUnfoldedBayes4"], "Unfolded (B-4)"},
  //           {h["HUnfoldedBayes10"], "Unfolded (B-10)"},
  //             {h["HUnfoldedBayes100"], "Unfolded (B-100)"}}, outputname+"cunfold_Iteration");
 
  // DrawThese({{h["HMCReco"], "MC Reco"},
  //       {h["HMCGen"], "MC Gen"},
  //         {h["HDataReco"], "Data"}}, fpdf);

  // DrawThese({{h["HMCReco_norm"], "MC Reco"},
  //       {h["HMCGen_norm"], "MC Gen"},
  //         {h["HDataReco_norm"], "Data"}}, fpdf);

  // DrawThese({{h["hrefpt_match_gen_jD"], "Gen p_{T} > 80 GeV"},
  //       {h["hrefpt_match_reco_jD"], "Reco p_{T} > 80 GeV"}}, fpdf, "Number of jet-D pairs vs. ref p_{T}");


  delete pa;
}

int main(int argc, char* argv[])
{
  if(argc == 3)
    return draw(argv[1], argv[2]);
  return 1;
}

void DrawThese(std::vector<std::pair<TH1D*, std::string>> tags, xjjroot::mypdf* f, std::string comment)
{
  double ymin = 1.e+10, ymax = -1.e+10;
  for(auto& t : tags)
    {
      auto thisymin = xjjana::gethminimum(t.first),
        thisymax = xjjana::gethmaximum(t.first);
      if(thisymin>0)
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
      t.first->SetMinimum(0);
      t.first->SetMaximum(ymax*1.5);
      leg->AddEntry(t.first, t.second.c_str(), "pl");
      t.first->Draw((count?"ple same":"ple"));
      count++;
    }
  leg->Draw();
  pa->drawtex(0.27, 0.85, 0.035, "cent");
  xjjroot::drawCMSleft();
  xjjroot::drawCMSright(Form("%s #sqrt{s_{NN}} = 5.02 TeV", pa->tag("ishi").c_str()));
  xjjroot::drawcomment(comment);
  f->write();

  f->prepare();
  c->SetLogy();
  count = 0;
  for(auto& t : tags)
    {
      t.first->SetMinimum(ymin/2.);
      t.first->SetMaximum(ymax*10);
      t.first->Draw((count?"ple same":"ple"));
      count++;
    }
  leg->Draw();
  pa->drawtex(0.27, 0.85, 0.035, "cent");
  xjjroot::drawCMSleft();
  xjjroot::drawCMSright(Form("%s #sqrt{s_{NN}} = 5.02 TeV", pa->tag("ishi").c_str()));
  xjjroot::drawcomment(comment);
  f->write();
}
