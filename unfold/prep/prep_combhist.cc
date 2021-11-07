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
  {"hratio_GenptRecophi_RecoptRecophi", kGray},
  {"hratio_GenptRecophi_RecoptRecophi_norm", kBlack},
  {"hratio_GenptGenphi_GenptRecophi", kGray},
  {"hratio_GenptGenphi_GenptRecophi_norm", kBlack},
};
xjjana::bins<float> vbDpt({6, 20, 30, 50});
void DrawThese(std::vector<std::pair<TH1D*, std::string>> vh, std::vector<std::string> tex, std::string collision, bool islogy, std::string forcestyle="");
void DrawAllpads(std::vector<std::pair<std::vector<TH1D*>, std::string>> vh, std::string collision, xjjroot::mypdf* fpdf, std::string forcestyle="");
int draw_ptdep(std::string var)
{
  // read histograms
  std::vector<std::string> names = {
    "HMCReco", "HMCGen", "HDataReco", 
    "HMCMatched", "HMCMatchedRecoptGenphi", "HMCMatchedGenptRecophi", "HMCMatchedGenptGenphi",
    "HDataReco_nojercorr", "hratio_GenptRecophi_RecoptRecophi", "hratio_GenptGenphi_GenptRecophi"
  };
  std::map<std::string, std::vector<TH1D*>> h_pp, h_PbPb;
  for(auto& t : names)
    {
      h_pp[t].resize(vbDpt.n());
      h_pp[t+"_norm"].resize(vbDpt.n());
      h_PbPb[t].resize(vbDpt.n());
      h_PbPb[t+"_norm"].resize(vbDpt.n());
    }
  for(int i=0; i<vbDpt.n(); i++)
    {
      std::string inputname_pp = "rootfiles/unfold_pp_jtpt80_jteta1.6_pt"+xjjc::number_remove_zero(vbDpt[i])+"-"+xjjc::number_remove_zero(vbDpt[i+1])+"_y1.2_"+var+".root";
      TFile* inf_pp = TFile::Open(inputname_pp.c_str());
      for(auto& t : names)
        {
          h_pp[t][i] = (TH1D*)inf_pp->Get(Form("%s-original", t.c_str()));
          h_pp[t][i]->SetName(Form("%s_pp_%d", h_pp[t][i]->GetName(), i));
          h_pp[t+"_norm"][i] = (TH1D*)inf_pp->Get(Form("%s_norm-original", t.c_str()));
          h_pp[t+"_norm"][i]->SetName(Form("%s_pp_%d", h_pp[t+"_norm"][i]->GetName(), i));
          xjjroot::sethempty(h_pp[t][i], 0, 0.05);
          xjjroot::sethempty(h_pp[t+"_norm"][i], 0, 0.05);
          int clr = xjjc::str_contains(t, "Data")?kBlack:colors[t],
            clr_norm = xjjc::str_contains(t, "Data")?kBlack:colors[t+"_norm"];
          xjjroot::setthgrstyle(h_pp[t][i], clr, 20, 0.8, clr, 1, 1);
          xjjroot::setthgrstyle(h_pp[t+"_norm"][i], clr_norm, 20, 0.8, clr_norm, 1, 1);
        }
      std::string inputname_PbPb = "rootfiles/unfold_PbPb_jtpt80_jteta1.6_pt"+xjjc::number_remove_zero(vbDpt[i])+"-"+xjjc::number_remove_zero(vbDpt[i+1])+"_y1.2_cent090_"+var+".root";
      TFile* inf_PbPb = TFile::Open(inputname_PbPb.c_str());
      for(auto& t : names)
        {
          h_PbPb[t][i] = (TH1D*)inf_PbPb->Get(Form("%s-original", t.c_str()));
          h_PbPb[t][i]->SetName(Form("%s_PbPb_%d", h_PbPb[t][i]->GetName(), i));
          h_PbPb[t+"_norm"][i] = (TH1D*)inf_PbPb->Get(Form("%s_norm-original", t.c_str()));
          h_PbPb[t+"_norm"][i]->SetName(Form("%s_PbPb_%d", h_PbPb[t+"_norm"][i]->GetName(), i));
          xjjroot::sethempty(h_PbPb[t][i], 0, 0.05);
          xjjroot::sethempty(h_PbPb[t+"_norm"][i], 0, 0.05);
          int clr = xjjc::str_contains(t, "Data")?xjjroot::mycolor_satmiddle2["red"]:colors[t],
            clr_norm = xjjc::str_contains(t, "Data")?xjjroot::mycolor_satmiddle2["red"]:colors[t+"_norm"];
          xjjroot::setthgrstyle(h_PbPb[t][i], clr, 20, 0.8, clr, 1, 1);
          xjjroot::setthgrstyle(h_PbPb[t+"_norm"][i], clr_norm, 20, 0.8, clr_norm, 1, 1);
        }
    }

  // draw
  xjjroot::setgstyle();
  // gStyle->SetLineScalePS(2);
  xjjroot::mypdf* fpdf = new xjjroot::mypdf("plots/unfold_ptdep_" + var + ".pdf", "c", 600*vbDpt.n(), 600);

  DrawAllpads({
      {h_PbPb["HDataReco_nojercorr"], "PbPb data (No JER corr)"},
        {h_pp["HDataReco_nojercorr"], "pp data (No JER corr)"},
          {h_pp["HMCGen"], "MC Gen"},
            }, "", fpdf);

  DrawAllpads({
      {h_PbPb["HDataReco"], "PbPb data (Correct JER)"},
        {h_pp["HDataReco"], "pp data (Correct JER)"},
          {h_pp["HMCGen"], "MC Gen"},
            }, "", fpdf);

  DrawAllpads({
      {h_PbPb["HDataReco_nojercorr_norm"], "PbPb data (No JER corr)"},
        {h_pp["HDataReco_nojercorr_norm"], "pp data (No JER corr)"},
          {h_pp["HMCGen_norm"], "MC Gen"},
            }, "", fpdf);

  DrawAllpads({
      {h_PbPb["HDataReco_norm"], "PbPb data (Correct JER)"},
        {h_pp["HDataReco_norm"], "pp data (Correct JER)"},
          {h_pp["HMCGen_norm"], "MC Gen"},
            }, "", fpdf);

  DrawAllpads({{h_pp["hratio_GenptRecophi_RecoptRecophi"], "N_{jet} normalized"},
        {h_pp["hratio_GenptRecophi_RecoptRecophi_norm"], "N_{jD} normalized"}}, 
    "pp", fpdf);

  DrawAllpads({{h_PbPb["hratio_GenptRecophi_RecoptRecophi"], "N_{jet} normalized"},
        {h_PbPb["hratio_GenptRecophi_RecoptRecophi_norm"], "N_{jD} normalized"}}, 
    "PbPb", fpdf);

  DrawAllpads({{h_pp["hratio_GenptGenphi_GenptRecophi"], "N_{jet} normalized"},
        {h_pp["hratio_GenptGenphi_GenptRecophi_norm"], "N_{jD} normalized"}}, 
    "pp", fpdf);

  DrawAllpads({{h_PbPb["hratio_GenptGenphi_GenptRecophi"], "N_{jet} normalized"},
        {h_PbPb["hratio_GenptGenphi_GenptRecophi_norm"], "N_{jD} normalized"}}, 
    "PbPb", fpdf);

  DrawAllpads({
          {h_pp["HMCMatchedGenptRecophi"], "Matched (Gen p_{T}, Reco #phi,#eta)"},
              {h_pp["HMCMatched"], "Matched (Reco p_{T}, Reco #phi,#eta)"},
                }, "pp", fpdf, "pe");

  DrawAllpads({
          {h_PbPb["HMCMatchedGenptRecophi"], "Matched (Gen p_{T}, Reco #phi,#eta)"},
              {h_PbPb["HMCMatched"], "Matched (Reco p_{T}, Reco #phi,#eta)"},
                }, "PbPb", fpdf, "pe");

  DrawAllpads({
      {h_pp["HMCGen"], "MC Gen"},
        {h_pp["HMCMatchedGenptGenphi"], "Matched (Gen p_{T}, Gen #phi,#eta)"},
          {h_pp["HMCMatchedGenptRecophi"], "Matched (Gen p_{T}, Reco #phi,#eta)"},
            {h_pp["HMCMatchedRecoptGenphi"], "Matched (Reco p_{T}, Gen #phi,#eta)"},
              {h_pp["HMCMatched"], "Matched (Reco p_{T}, Reco #phi,#eta)"},
                {h_pp["HMCReco"], "MC Reco"}}, 
    "pp", fpdf, "pe");

  DrawAllpads({
      {h_pp["HMCGen_norm"], "MC Gen"},
        {h_pp["HMCMatchedGenptGenphi_norm"], "Matched (Gen p_{T}, Gen #phi,#eta)"},
          {h_pp["HMCMatchedGenptRecophi_norm"], "Matched (Gen p_{T}, Reco #phi,#eta)"},
            {h_pp["HMCMatchedRecoptGenphi_norm"], "Matched (Reco p_{T}, Gen #phi,#eta)"},
              {h_pp["HMCMatched_norm"], "Matched (Reco p_{T}, Reco #phi,#eta)"},
                {h_pp["HMCReco_norm"], "MC Reco"}}, 
    "pp", fpdf, "pe");

  DrawAllpads({
      {h_PbPb["HMCGen"], "MC Gen"},
        {h_PbPb["HMCMatchedGenptGenphi"], "Matched (Gen p_{T}, Gen #phi,#eta)"},
          {h_PbPb["HMCMatchedGenptRecophi"], "Matched (Gen p_{T}, Reco #phi,#eta)"},
            {h_PbPb["HMCMatchedRecoptGenphi"], "Matched (Reco p_{T}, Gen #phi,#eta)"},
              {h_PbPb["HMCMatched"], "Matched (Reco p_{T}, Reco #phi,#eta)"},
                {h_PbPb["HMCReco"], "MC Reco"}}, 
    "PbPb", fpdf, "pe");

  DrawAllpads({
      {h_PbPb["HMCGen_norm"], "MC Gen"},
        {h_PbPb["HMCMatchedGenptGenphi_norm"], "Matched (Gen p_{T}, Gen #phi,#eta)"},
          {h_PbPb["HMCMatchedGenptRecophi_norm"], "Matched (Gen p_{T}, Reco #phi,#eta)"},
            {h_PbPb["HMCMatchedRecoptGenphi_norm"], "Matched (Reco p_{T}, Gen #phi,#eta)"},
              {h_PbPb["HMCMatched_norm"], "Matched (Reco p_{T}, Reco #phi,#eta)"},
                {h_PbPb["HMCReco_norm"], "MC Reco"}}, 
    "PbPb", fpdf, "pe");

  fpdf->close();

  // DrawAllpads({{h_pp["HMCMatched"], "MC Matched"},
  //       {h["HMCGen"], "MC Gen"},
  //         {h["HDataReco_nojercorr"], "Data (No JER corr)"}}, fpdf);

  // DrawAllpads({{h["HMCMatched"], "MC Matched"},
  //       {h["HMCGen"], "MC Gen"},
  //         {h["HDataReco"], "Data (Correct JER)"}}, fpdf);

  // DrawAllpads({{h["HMCMatched_norm"], "MC Matched"},
  //       {h["HMCGen_norm"], "MC Gen"},
  //         {h["HDataReco_nojercorr_norm"], "Data (No JER corr)"}}, fpdf);

  // DrawAllpads({{h["HMCMatched_norm"], "MC Matched"},
  //       {h["HMCGen_norm"], "MC Gen"},
  //         {h["HDataReco_norm"], "Data (Correct JER)"}}, fpdf);

  // DrawAllpads({{h["hrefpt_match_gen"], "Gen p_{T} > 80 GeV"},
  //       {h["hrefpt_match_reco"], "Reco p_{T} > 80 GeV"}}, fpdf, "Number of jets vs. ref p_{T}");
 
  // DrawAllpads({{h["HMCMatched"], "Matched (Reco p_{T}, Reco #phi,#eta)"},
  //       {h["HMCMatchedRecoptGenphi"], "Matched (Reco p_{T}, Gen #phi,#eta)"},
  //         {h["HMCMatchedGenptRecophi"], "Matched (Gen p_{T}, Reco #phi,#eta)"},
  //           {h["HMCMatchedGenptGenphi"], "Matched (Gen p_{T}, Gen #phi,#eta)"},
  //             }, fpdf);

  // DrawAllpads({
  //     {h["HMCGen"], "MC Gen"},
  //       {h["HMCReco"], "MC Reco"},
  //         }, fpdf);

  // DrawAllpads({{h["HDataReco"], "Data"},
  //       {h["HUnfoldedBayes10"], "Data Unfolded (B-10)"}}, outputname+"cunfold_HDataReco-HUnfoldedBayes10");

  // DrawAllpads({{h["HDataReco"], "Data"},
  //       {h["HMCReco"], "MC Reco"},
  //         {h["HMCGen"], "MC Gen"},
  //           {h["HUnfoldedBayes10"], "Data Unfolded (B-10)"}}, outputname+"cunfold_All");

  // DrawAllpads({{h["HUnfoldedBayes1"], "Unfolded (B-1)"},
  //       {h["HUnfoldedBayes2"], "Unfolded (B-2)"},
  //         {h["HUnfoldedBayes4"], "Unfolded (B-4)"},
  //           {h["HUnfoldedBayes10"], "Unfolded (B-10)"},
  //             {h["HUnfoldedBayes100"], "Unfolded (B-100)"}}, outputname+"cunfold_Iteration");
 
  // DrawAllpads({{h["HMCReco"], "MC Reco"},
  //       {h["HMCGen"], "MC Gen"},
  //         {h["HDataReco"], "Data"}}, fpdf);

  // DrawAllpads({{h["HMCReco_norm"], "MC Reco"},
  //       {h["HMCGen_norm"], "MC Gen"},
  //         {h["HDataReco_norm"], "Data"}}, fpdf);

  // DrawAllpads({{h["hrefpt_match_gen_jD"], "Gen p_{T} > 80 GeV"},
  //       {h["hrefpt_match_reco_jD"], "Reco p_{T} > 80 GeV"}}, fpdf, "Number of jet-D pairs vs. ref p_{T}");

}

int main(int argc, char* argv[])
{
  if(argc == 2)
    return draw_ptdep(argv[1]);
  return 1;
}

void DrawAllpads(std::vector<std::pair<std::vector<TH1D*>, std::string>> vh, std::string collision, xjjroot::mypdf* fpdf, std::string forcestyle)
{
  fpdf->prepare();
  for(int p=0; p<vbDpt.n(); p++)
    {
      TPad* pad = new TPad(Form("p%s%d", xjjc::currenttime().c_str(), p), "", p*1.0/vbDpt.n(), 0., (p+1)*1.0/vbDpt.n(), 1.); pad->Draw(); pad->cd();
      pad->SetLogy(0);
      std::vector<std::string> vtex = {Form("%s < p_{T}^{D} < %s GeV", xjjc::number_remove_zero(vbDpt[p]).c_str(), xjjc::number_remove_zero(vbDpt[p+1]).c_str())};
      std::vector<std::pair<TH1D*, std::string>> vhh;
      for(auto& h : vh)
        vhh.push_back({h.first[p], h.second});
      DrawThese(vhh, vtex, collision, false, forcestyle); // linear
      fpdf->getc()->cd();
    }
  fpdf->write();
  fpdf->prepare();
  for(int p=0; p<vbDpt.n(); p++)
    {
      TPad* pad = new TPad(Form("p%s%d", xjjc::currenttime().c_str(), p), "", p*1.0/vbDpt.n(), 0., (p+1)*1.0/vbDpt.n(), 1.); pad->Draw(); pad->cd();
      pad->SetLogy(1);
      std::vector<std::string> vtex = {Form("%s < p_{T}^{D} < %s GeV", xjjc::number_remove_zero(vbDpt[p]).c_str(), xjjc::number_remove_zero(vbDpt[p+1]).c_str())};
      std::vector<std::pair<TH1D*, std::string>> vhh;
      for(auto& h : vh)
        vhh.push_back({h.first[p], h.second});
      DrawThese(vhh, vtex, collision, true, forcestyle); // log
      fpdf->getc()->cd();
    }
  fpdf->write();
}

void DrawThese(std::vector<std::pair<TH1D*, std::string>> vh, std::vector<std::string> tex, std::string collision, bool islogy, std::string forcestyle)
{
  TLegend* leg = new TLegend(0.50, 0.85-0.035*vh.size(), 0.70, 0.85);
  xjjroot::setleg(leg, 0.032);
  std::vector<std::string> vtex = {"|#eta^{jet}| < 1.6", "p_{T}^{jet} > 80 GeV", "|#eta^{D}| < 1.2"};
  vtex.insert(vtex.end(), tex.begin(), tex.end());
  double ymin = 1.e+10, ymax = -1.e+10;
  for(auto& h : vh)
    {
      auto thisymin = xjjana::gethminimum(h.first),
        thisymax = xjjana::gethmaximum(h.first);
      if(thisymin>0)
        ymin = std::min(ymin, thisymin);
      ymax = std::max(ymax, thisymax);
      leg->AddEntry(h.first, h.second.c_str(), "l");
    }
  if(islogy)
    for(auto& h : vh)
      xjjana::sethabsminmax(h.first, ymin*0.5, ymax*10.);
  else
    for(auto& h : vh)
      xjjana::sethabsminmax(h.first, 0, ymax*1.5);

  vh[0].first->Draw("AXIS");

  for(auto& h : vh)
    {
      auto gr = xjjana::histtograph(h.first, Form("gr_%s", h.first->GetName()));
      if(forcestyle=="")
        {
          if(xjjc::str_contains(h.first->GetName(), "Data") || xjjc::str_contains(h.first->GetName(), "hratio"))
            gr->Draw("pe same");
          else
            gr->Draw("LX0 same");
        }
      else
        {
          gr->Draw(Form("%s same", forcestyle.c_str()));
        }
    }
  leg->Draw();
  int iy = -1; for(auto& t : vtex)
                 xjjroot::drawtex(0.25, 0.85-0.035*(++iy), t.c_str(), 0.032, 13);
  xjjroot::drawCMS("Preliminary", Form("%s #sqrt{s_{NN}} = 5.02 TeV", collision.c_str()));
}
