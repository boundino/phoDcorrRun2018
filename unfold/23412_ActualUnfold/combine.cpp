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
#include "bins.h"

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
  {"HClosure_before", kGray},
  {"HClosure", kBlack},
};
void DrawThese(std::vector<std::pair<TH1D*, std::string>> vh, std::vector<std::string> tex, std::string collision, bool islogy, std::vector<std::string> forcestyle, float forceymin = -1., float forceymax = -1.);
void DrawAllpads(std::vector<std::pair<std::string, std::string>> vh, std::string collision, xjjroot::mypdf* fpdf, float forceymin = -1., float forceymax = -1.);
Djet::param* pa;
xjjana::bins<float> vbDpt({6, 20, 30, 50}),
  vbcent({0, 10, 30, 50});

std::map<std::string, std::vector<TH1D*>> h;
std::map<std::string, std::vector<TGraphErrors*>> gr;
TH1D* getandscaleh(TFile* inf, std::string name, std::string coll, std::string var, std::string unique, bool scale = true);
int draw(std::string var)
{
  std::vector<std::string> names = {
    "HMCMeasured", "HMCTruth", "HInput", 
    "HUnfoldedBayes1", "HUnfoldedBayes2", "HUnfoldedBayes4", "HUnfoldedBayes10",  "HUnfoldedBayes100",
    "HUnfoldedInvert", 
    "HUnfoldedSVD1", "HUnfoldedSVD2", "HUnfoldedSVD4",
    "HClosure", "HClosure_before"
  };
  std::vector<std::string> vkinematics;
  for(int i=0; i<vbDpt.n(); i++) vkinematics.push_back(Form("%.0f-%.0f_y1.2_cent090", vbDpt[i], vbDpt[i+1]));
  for(int i=0; i<vbcent.n(); i++) vkinematics.push_back(Form("6-50_y1.2_cent%.0f%.0f", vbcent[i], vbcent[i+1]));
  std::vector<std::vector<TGraph*>> grBayes_pp(vkinematics.size()), grBayes_PbPb(vkinematics.size()), grSVD_pp(vkinematics.size()), grSVD_PbPb(vkinematics.size());
  for(int i=0; i<vkinematics.size(); i++)
    {
      std::string i_pp = xjjc::str_erasestar(vkinematics[i], "_cent*");
      TFile* inf_pp = TFile::Open(std::string("Output/Result_Unfolded_pp_jtpt80_jteta1.6_pt"+i_pp+"_"+var+".root").c_str());
      TFile* inf_PbPb = TFile::Open(std::string("Output/Result_Unfolded_PbPb_jtpt80_jteta1.6_pt"+vkinematics[i]+"_"+var+".root").c_str());
      TFile* inf_closure_pp = TFile::Open(std::string("Output/Result_Closure_pp_jtpt80_jteta1.6_pt"+i_pp+"_"+var+".root").c_str());
      TFile* inf_closure_PbPb = TFile::Open(std::string("Output/Result_Closure_PbPb_jtpt80_jteta1.6_pt"+vkinematics[i]+"_"+var+".root").c_str());
      for(auto& t : names)
        {
          bool doscale = !(t=="HClosure"||t=="HClosure_before");
          h[t+"_pp"].push_back(getandscaleh(inf_pp, t, "pp", var, Form("%d", i), doscale));
          xjjroot::sethempty(h[t+"_pp"][i], 0, 0);
          h[t+"_pp_comp"].push_back((TH1D*)h[t+"_pp"][i]->Clone(Form("%s_comp", h[t+"_pp"][i]->GetName())));
          xjjroot::setthgrstyle(h[t+"_pp_comp"][i], kBlack, 20, 0.8, kBlack, 1, 1);
          h[t+"_PbPb"].push_back(getandscaleh(inf_PbPb, t, "PbPb", var, Form("%d", i), doscale));
          xjjroot::sethempty(h[t+"_PbPb"][i], 0, 0);
          h[t+"_PbPb_comp"].push_back((TH1D*)h[t+"_PbPb"][i]->Clone(Form("%s_comp", h[t+"_PbPb"][i]->GetName())));
          xjjroot::setthgrstyle(h[t+"_PbPb_comp"][i], kRed+2, 20, 0.8, kRed+2, 1, 1);
          h[t+"_closure_pp"].push_back(getandscaleh(inf_closure_pp, t, "closure_pp", var, Form("%d", i), doscale));
          h[t+"_closure_PbPb"].push_back(getandscaleh(inf_closure_PbPb, t, "closure_PbPb", var, Form("%d", i), doscale));
        }
      // std::cout<<h["HInput_PbPb"][i]<<"  "<<h["HInput_pp"][i]<<"  "<<h["HUnfoldedBayes4_PbPb"][i]<<"  "<<h["HUnfoldedBayes4_pp"][i]<<std::endl;
      auto hh = (TH1D*)h["HInput_PbPb"][i]->Clone(Form("HRatio_%d", i));
      hh->Divide(h["HInput_pp"][i]);
      xjjroot::setthgrstyle(hh, kGray, 20, 0.8, kGray, 1, 1);
      hh->GetYaxis()->SetTitle("PbPb / pp");
      h["HRatio"].push_back(hh);
      hh = (TH1D*)h["HUnfoldedBayes4_PbPb"][i]->Clone(Form("HRatioUnfoldedBayes4_%d", i));
      hh->Divide(h["HUnfoldedBayes4_pp"][i]);
      xjjroot::setthgrstyle(hh, kBlack, 20, 0.8, kBlack, 1, 1);
      hh->GetYaxis()->SetTitle("PbPb / pp");
      h["HRatio_HUnfolded"].push_back(hh);

      for(int k=0; k<Djet::bins[var].size()-1; k++)
        {
          auto g = (TGraph*)inf_pp->Get(Form("grBayes%d", k)); g->SetName(Form("%s_pp_%d", g->GetName(), i));
          xjjroot::setthgrstyle(g, xjjroot::colorlist_bw[k], 20, 0.7, xjjroot::colorlist_bw[k], 1, 1);
          grBayes_pp[i].push_back(g);
          g = (TGraph*)inf_PbPb->Get(Form("grBayes%d", k)); g->SetName(Form("%s_PbPb_%d", g->GetName(), i));
          xjjroot::setthgrstyle(g, xjjroot::colorlist_bw[k], 20, 0.7, xjjroot::colorlist_bw[k], 1, 1);
          grBayes_PbPb[i].push_back(g);
          g = (TGraph*)inf_pp->Get(Form("grSVD%d", k)); g->SetName(Form("%s_pp_%d", g->GetName(), i));
          xjjroot::setthgrstyle(g, xjjroot::colorlist_bw[k], 20, 0.7, xjjroot::colorlist_bw[k], 1, 1);
          grSVD_pp[i].push_back(g);
          g = (TGraph*)inf_PbPb->Get(Form("grSVD%d", k)); g->SetName(Form("%s_PbPb_%d", g->GetName(), i));
          xjjroot::setthgrstyle(g, xjjroot::colorlist_bw[k], 20, 0.7, xjjroot::colorlist_bw[k], 1, 1);
          grSVD_PbPb[i].push_back(g);
        }
    }

  // return 2;

  // TH2D* HMCResponse = (TH2D*)inf->Get("HMCResponse");
  // xjjroot::sethempty(HMCResponse, 0, 0);
  // HMCResponse->GetXaxis()->SetNdivisions(-1*xjjana::gethXn(h[names.front()])-100);
  // HMCResponse->GetYaxis()->SetNdivisions(-1*xjjana::gethXn(h[names.front()])-100);
  // TH2D* HMCResponse_norm = (TH2D*)HMCResponse->Clone("HMCResponse_norm");
  // xjjana::normTH2(HMCResponse_norm, "y");
  // xjjroot::sethempty(HMCResponse_norm, 0, 0);
  // HMCResponse_norm->GetXaxis()->SetNdivisions(-1*xjjana::gethXn(h[names.front()])-100);
  // HMCResponse_norm->GetYaxis()->SetNdivisions(-1*xjjana::gethXn(h[names.front()])-100);

  TH2D* hempty_Bayes = new TH2D("hempty_Bayes", ";Iterations;", 10, 0.6, 300, 10, 0, 2);
  xjjroot::sethempty(hempty_Bayes);
  TH2D* hempty_SVD = new TH2D("hempty_SVD", ";Iterations;", 10, 0, 10, 10, 0, 2);
  xjjroot::sethempty(hempty_SVD);

  xjjroot::setgstyle(1);
  gStyle->SetPaintTextFormat("1.2f");
  gStyle->SetLineScalePS(2);
  xjjroot::mypdf* fpdf = new xjjroot::mypdf("plots/combine/c_" + var + ".pdf", "c", 1800, 600);

  // fpdf->getc()->SetGrid();
  // fpdf->prepare();
  // HMCResponse->Draw("col TEXT");
  // xjjroot::drawCMSleft();
  // xjjroot::drawCMSright(Form("%s #sqrt{s_{NN}} = 5.02 TeV", pa->tag("ishi").c_str()));
  // fpdf->write();

  // fpdf->prepare();
  // HMCResponse_norm->Draw("col TEXT");
  // xjjroot::drawCMSleft();
  // xjjroot::drawCMSright(Form("%s #sqrt{s_{NN}} = 5.02 TeV", pa->tag("ishi").c_str()));
  // fpdf->write();

  fpdf->getc()->SetGrid(0);

  DrawAllpads({{"HInput_PbPb_comp:pe", "PbPb (Not unfolded)"},
        {"HInput_pp_comp:pe", "pp (Not unfolded)"},
          {"HMCTruth_pp:lX0", "PYTHIA 8"},
            }, "", fpdf);
  
  DrawAllpads({{"HUnfoldedBayes4_PbPb_comp:pe", "PbPb (Unfolded B-4)"},
        {"HUnfoldedBayes4_pp_comp:pe", "pp (Unfolded B-4)"},
          {"HMCTruth_pp:lX0", "PYTHIA 8"},
            }, "", fpdf);
  
  DrawAllpads({{"HRatio:pe", "Not unfolded"},
        {"HRatio_HUnfolded:pe", "Unfolded (B-4)"},
          }, "", fpdf);
  
  DrawAllpads({{"HMCMeasured_pp:pe", "MC Reco"},
        {"HMCTruth_pp:pe", "MC Gen"},
          {"HInput_pp:pe", "pp"}
    }, "pp", fpdf);
  
  DrawAllpads({{"HMCMeasured_pp:pe", "MC Reco"},
        {"HMCTruth_pp:pe", "MC Gen"},
          {"HInput_pp:pe", "pp"},
            {"HUnfoldedBayes4_pp:pe", "pp (Unfolded B-4)"},
              }, "pp", fpdf);
  
  DrawAllpads({{"HMCTruth_pp:pe", "MC Gen"},
        {"HInput_pp:pe", "pp"},
          {"HUnfoldedBayes1_pp:pe", "pp (Unfolded B-1)"},
            {"HUnfoldedBayes2_pp:pe", "pp (Unfolded B-2)"},
              {"HUnfoldedBayes4_pp:pe", "pp (Unfolded B-4)"},
                {"HUnfoldedBayes10_pp:pe", "pp (Unfolded B-10)"},
                  {"HUnfoldedBayes100_pp:pe", "pp (Unfolded B-100)"},
                    }, "pp", fpdf);
  
  DrawAllpads({{"HMCMeasured_PbPb:pe", "MC Reco"},
        {"HMCTruth_PbPb:pe", "MC Gen"},
          {"HInput_PbPb:pe", "PbPb"}
    }, "PbPb", fpdf);
  
  DrawAllpads({{"HMCMeasured_PbPb:pe", "MC Reco"},
        {"HMCTruth_PbPb:pe", "MC Gen"},
          {"HInput_PbPb:pe", "PbPb"},
            {"HUnfoldedBayes4_PbPb:pe", "PbPb (Unfolded B-4)"},
              }, "PbPb", fpdf);
  
  DrawAllpads({{"HMCTruth_PbPb:pe", "MC Gen"},
        {"HInput_PbPb:pe", "PbPb"},
          {"HUnfoldedBayes1_PbPb:pe", "PbPb (Unfolded B-1)"},
            {"HUnfoldedBayes2_PbPb:pe", "PbPb (Unfolded B-2)"},
              {"HUnfoldedBayes4_PbPb:pe", "PbPb (Unfolded B-4)"},
                {"HUnfoldedBayes10_PbPb:pe", "PbPb (Unfolded B-10)"},
                  {"HUnfoldedBayes100_PbPb:pe", "PbPb (Unfolded B-100)"},
                    }, "PbPb", fpdf);
  
  DrawAllpads({{"HClosure_before_closure_pp:pe", "MC Reco / Truth"},
        {"HClosure_closure_pp:pe", "MC Unfolded / Truth"},
          }, "pp", fpdf, 0.4, 1.6);
  
  DrawAllpads({{"HClosure_before_closure_PbPb:pe", "MC Reco / Truth"},
        {"HClosure_closure_PbPb:pe", "MC Unfolded / Truth"},
          }, "PbPb", fpdf, 0.4, 1.6);

  fpdf->prepare();
  for(int p=0; p<vbDpt.n(); p++)
    {
      TPad* pad = new TPad(Form("p%s%d", xjjc::currenttime().c_str(), p), "", p*1.0/(vbDpt.n()), 0., (p+1)*1.0/(vbDpt.n()), 1.); pad->Draw(); pad->cd();
      pad->SetLogx();
      std::vector<std::string> vtex = {"|#eta^{jet}| < 1.6", "p_{T}^{jet} > 80 GeV", "|#eta^{D}| < 1.2", 
                                       Form("%s < p_{T}^{D} < %s GeV", xjjc::number_remove_zero(vbDpt[p]).c_str(), xjjc::number_remove_zero(vbDpt[p+1]).c_str())};
      hempty_Bayes->Draw("AXIS");
      for(int k=0; k<Djet::bins[var].size()-1; k++)
        grBayes_pp[p][k]->Draw("pl same");
      int iy = -1; for(auto& t : vtex)
                     xjjroot::drawtex(0.25, 0.85-0.035*(++iy), t.c_str(), 0.032, 13);
      xjjroot::drawCMS("Preliminary", "pp #sqrt{s_{NN}} = 5.02 TeV");
      fpdf->getc()->cd();
    }
  fpdf->write();
  
  fpdf->prepare();
  for(int p=0; p<vbDpt.n(); p++)
    {
      TPad* pad = new TPad(Form("p%s%d", xjjc::currenttime().c_str(), p), "", p*1.0/(vbDpt.n()), 0., (p+1)*1.0/(vbDpt.n()), 1.); pad->Draw(); pad->cd();
      pad->SetLogx();
      std::vector<std::string> vtex = {"|#eta^{jet}| < 1.6", "p_{T}^{jet} > 80 GeV", "|#eta^{D}| < 1.2", 
                                       Form("%s < p_{T}^{D} < %s GeV", xjjc::number_remove_zero(vbDpt[p]).c_str(), xjjc::number_remove_zero(vbDpt[p+1]).c_str())};
      hempty_Bayes->Draw("AXIS");
      for(int k=0; k<Djet::bins[var].size()-1; k++)
        grBayes_PbPb[p][k]->Draw("pl same");
      int iy = -1; for(auto& t : vtex)
                     xjjroot::drawtex(0.25, 0.85-0.035*(++iy), t.c_str(), 0.032, 13);
      xjjroot::drawCMS("Preliminary", "PbPb #sqrt{s_{NN}} = 5.02 TeV");
      fpdf->getc()->cd();
    }
  fpdf->write();
  
  fpdf->prepare();
  for(int p=0; p<vbcent.n(); p++)
    {
      TPad* pad = new TPad(Form("p%s%d", xjjc::currenttime().c_str(), p), "", p*1.0/(vbcent.n()), 0., (p+1)*1.0/(vbcent.n()), 1.); pad->Draw(); pad->cd();
      pad->SetLogx();
      std::vector<std::string> vtex = {"|#eta^{jet}| < 1.6", "p_{T}^{jet} > 80 GeV", "|#eta^{D}| < 1.2", 
                                       Form("Cent. %s-%s%%", xjjc::number_remove_zero(vbcent[p]).c_str(), xjjc::number_remove_zero(vbcent[p+1]).c_str())};
      hempty_Bayes->Draw("AXIS");
      for(int k=0; k<Djet::bins[var].size()-1; k++)
        grBayes_PbPb[vbDpt.n()+p][k]->Draw("pl same");
      int iy = -1; for(auto& t : vtex)
                     xjjroot::drawtex(0.25, 0.85-0.035*(++iy), t.c_str(), 0.032, 13);
      xjjroot::drawCMS("Preliminary", "PbPb #sqrt{s_{NN}} = 5.02 TeV");
      fpdf->getc()->cd();
    }
  fpdf->write();
  
  fpdf->close();

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc == 2)
    return draw(argv[1]);
  return 1;
}

TH1D* getandscaleh(TFile* inf, std::string name, std::string coll, std::string var, std::string unique, bool scale)
{
  auto thish = (TH1D*)inf->Get(name.c_str());
  // std::cout<<name<<" "<<thish<<std::endl;
  if(scale) thish->Scale(1./thish->Integral());
  thish = xjjana::changebin(thish, Djet::bins[var], Form("%s_%s_%s", thish->GetName(), coll.c_str(), unique.c_str()));
  thish->GetXaxis()->SetTitle(Djet::vartex[var].c_str());
  xjjana::bins<float> vb(Djet::bins[var], false);
  if(scale)
    {
      for(int k=0; k<thish->GetXaxis()->GetNbins(); k++)
        {
          float bin_width = vb.width(k); // dphi
          if(var=="dr") bin_width = vb.area(k);
          thish->SetBinContent(k+1, thish->GetBinContent(k+1) / bin_width);
          thish->SetBinError(k+1, thish->GetBinError(k+1) / bin_width);
        }
    }
  xjjroot::sethempty(thish, 0, 0);
  xjjroot::setthgrstyle(thish, colors[name], 20, 0.8, colors[name], 1, 1);
  return thish;
}

void DrawThese(std::vector<std::pair<TH1D*, std::string>> vh, std::vector<std::string> tex, std::string collision, bool islogy, std::vector<std::string> forcestyle, float forceymin, float forceymax)
{
  TLegend* leg = new TLegend(0.50, 0.85-0.035*vh.size(), 0.70, 0.85);
  xjjroot::setleg(leg, 0.032);
  std::vector<std::string> vtex = {"|#eta^{jet}| < 1.6", "p_{T}^{jet} > 80 GeV", "|#eta^{D}| < 1.2"};
  vtex.insert(vtex.end(), tex.begin(), tex.end());
  double ymin = 1.e+10, ymax = -1.e+10;
  for(auto& hh : vh)
    {
      auto thisymin = xjjana::gethminimum(hh.first),
        thisymax = xjjana::gethmaximum(hh.first);
      if(thisymin>0 || !islogy)
        ymin = std::min(ymin, thisymin);
      ymax = std::max(ymax, thisymax);
      leg->AddEntry(hh.first, hh.second.c_str(), "l");
    }
  if(islogy)
    for(auto& hh : vh)
      xjjana::sethabsminmax(hh.first, forceymin>0?forceymin:(ymin*0.5), forceymax>0?forceymax:(ymax*10.));
  else
    for(auto& hh : vh)
      xjjana::sethabsminmax(hh.first, forceymin>0?forceymin:0, forceymax>0?forceymax:(ymax*1.5));

  vh[0].first->Draw("AXIS");

  for(int i=0; i<vh.size(); i++)
    {
      auto hh = vh[i].first;
      auto gr = xjjana::histtograph(hh, Form("gr_%s", hh->GetName()));
      if(forcestyle[i]=="")
        gr->Draw("pe same");
      else
        gr->Draw(Form("%s same", forcestyle[i].c_str()));
    }
  leg->Draw();
  int iy = -1; for(auto& t : vtex)
                 xjjroot::drawtex(0.25, 0.85-0.035*(++iy), t.c_str(), 0.032, 13);
  xjjroot::drawCMS("Preliminary", Form("%s #sqrt{s_{NN}} = 5.02 TeV", collision.c_str()));
}

void DrawAllpads(std::vector<std::pair<std::string, std::string>> vname, std::string collision, xjjroot::mypdf* fpdf, float forceymin, float forceymax)
{
  std::vector<std::string> names, styles;
  for(auto& t : vname)
    {
      names.push_back(xjjc::str_divide(t.first, ":")[0]);
      styles.push_back(xjjc::str_divide(t.first, ":")[1]);
    }
  fpdf->prepare();
  for(int p=0; p<vbDpt.n(); p++)
    {
      TPad* pad = new TPad(Form("p%s%d", xjjc::currenttime().c_str(), p), "", p*1.0/(vbDpt.n()), 0., (p+1)*1.0/(vbDpt.n()), 1.); pad->Draw(); pad->cd();
      pad->SetLogy(0);
      std::vector<std::string> vtex = {Form("%s < p_{T}^{D} < %s GeV", xjjc::number_remove_zero(vbDpt[p]).c_str(), xjjc::number_remove_zero(vbDpt[p+1]).c_str())};
      std::vector<std::pair<TH1D*, std::string>> vhh;
      for(int i=0; i<vname.size(); i++)
        vhh.push_back({h[names[i]][p], vname[i].second});
      DrawThese(vhh, vtex, collision, false, styles, forceymin, forceymax); // linear
      fpdf->getc()->cd();
    }
  fpdf->write();
  fpdf->prepare();
  for(int p=0; p<vbDpt.n(); p++)
    {
      TPad* pad = new TPad(Form("p%s%d", xjjc::currenttime().c_str(), p), "", p*1.0/(vbDpt.n()), 0., (p+1)*1.0/(vbDpt.n()), 1.); pad->Draw(); pad->cd();
      pad->SetLogy(1);
      std::vector<std::string> vtex = {Form("%s < p_{T}^{D} < %s GeV", xjjc::number_remove_zero(vbDpt[p]).c_str(), xjjc::number_remove_zero(vbDpt[p+1]).c_str())};
      std::vector<std::pair<TH1D*, std::string>> vhh;
      for(int i=0; i<vname.size(); i++)
        vhh.push_back({h[names[i]][p], vname[i].second});
      DrawThese(vhh, vtex, collision, true, styles, forceymin, forceymax); // log
      fpdf->getc()->cd();
    }
  fpdf->write();

  if(collision == "pp") return;

  fpdf->prepare();
  for(int p=0; p<vbcent.n(); p++)
    {
      TPad* pad = new TPad(Form("p%s%d", xjjc::currenttime().c_str(), p), "", p*1.0/(vbcent.n()), 0., (p+1)*1.0/(vbcent.n()), 1.); pad->Draw(); pad->cd();
      pad->SetLogy(0);
      std::vector<std::string> vtex = {Form("Cent. %s-%s%%", xjjc::number_remove_zero(vbcent[p]).c_str(), xjjc::number_remove_zero(vbcent[p+1]).c_str())};
      std::vector<std::pair<TH1D*, std::string>> vhh;
      for(int i=0; i<vname.size(); i++)
        vhh.push_back({h[names[i]][p+vbDpt.n()], vname[i].second});
      DrawThese(vhh, vtex, collision, false, styles, forceymin, forceymax); // linear
      fpdf->getc()->cd();
    }
  fpdf->write();
  fpdf->prepare();
  for(int p=0; p<vbcent.n(); p++)
    {
      TPad* pad = new TPad(Form("p%s%d", xjjc::currenttime().c_str(), p), "", p*1.0/(vbcent.n()), 0., (p+1)*1.0/(vbcent.n()), 1.); pad->Draw(); pad->cd();
      pad->SetLogy(1);
      std::vector<std::string> vtex = {Form("Cent. %s-%s%%", xjjc::number_remove_zero(vbcent[p]).c_str(), xjjc::number_remove_zero(vbcent[p+1]).c_str())};
      std::vector<std::pair<TH1D*, std::string>> vhh;
      for(int i=0; i<vname.size(); i++)
        vhh.push_back({h[names[i]][p+vbDpt.n()], vname[i].second});
      DrawThese(vhh, vtex, collision, true, styles, forceymin, forceymax); // log
      fpdf->getc()->cd();
    }
  fpdf->write();
}
