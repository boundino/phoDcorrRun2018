#include <TFile.h>
#include <TH1D.h>
#include <TLegend.h>
#include <string>
#include <vector>

#include "xjjanauti.h"
#include "xjjcuti.h"
#include "xjjrootuti.h"
#include "xjjmypdf.h"

#include "ptdif.h"

namespace ptdif
{
  void drawthese(xjjroot::mypdf* pdf, std::vector<TH1D*> vh, TLegend* leg,
                 std::string tex, std::string collision, bool islogy=false, std::string comment="");
}

int ptdif_drawhist(std::string inputname, std::string outsubdir, std::string var)
{
  std::string thi = xjjc::str_contains(outsubdir, "PbPb")?"PbPb":"pp";
  TFile* inf = TFile::Open(inputname.c_str());
  auto hgen = xjjc::array2d<TH1D*>(vbDpt.n(), vbpt.n()), href = xjjc::array2d<TH1D*>(vbDpt.n(), vbpt.n()),
    hgen_norm = xjjc::array2d<TH1D*>(vbDpt.n(), vbpt.n()), href_norm = xjjc::array2d<TH1D*>(vbDpt.n(), vbpt.n());
  TLegend* leg = new TLegend(0.50, 0.85-0.035*vbpt.n(), 0.75, 0.85);
  xjjroot::setleg(leg, 0.032);
  for(int p=0; p<vbDpt.n(); p++)
    {
      for(int i=0; i<vbpt.n(); i++)
        {
          hgen[p][i] = (TH1D*)inf->Get(Form("hgen_%d_%.0f_%.0f", p, vbpt[i], vbpt[i+1]));
          hgen_norm[p][i] = (TH1D*)hgen[p][i]->Clone(Form("%s_norm", hgen[p][i]->GetName()));
          double norm = (var=="dr"?xjjana::integral_area(hgen[p][i]):hgen[p][i]->Integral("width"));
          if(norm>0) hgen_norm[p][i]->Scale(1./norm);
          hgen_norm[p][i]->GetYaxis()->SetTitle(xjjc::str_replaceall(hgen[p][i]->GetYaxis()->GetTitle(), "jet", "jD").c_str());

          href[p][i] = (TH1D*)inf->Get(Form("href_%d_%.0f_%.0f", p, vbpt[i], vbpt[i+1]));
          href_norm[p][i] = (TH1D*)href[p][i]->Clone(Form("%s_norm", href[p][i]->GetName()));
          norm = (var=="dr"?xjjana::integral_area(href[p][i]):href[p][i]->Integral("width"));
          if(norm>0) href_norm[p][i]->Scale(1./norm);
          href_norm[p][i]->GetYaxis()->SetTitle(xjjc::str_replaceall(href[p][i]->GetYaxis()->GetTitle(), "jet", "jD").c_str());
        }
      for(int i=0; i<vbpt.n(); i++)
        {
          auto color = i<vbpt.n()-1?xjjroot::colorlist_middle[i]:kBlack;
          xjjroot::sethempty(hgen[p][i], 0, 0.2); hgen[p][i]->GetXaxis()->SetNdivisions(505);
          xjjroot::setthgrstyle(hgen[p][i], color, 20, 0.9, color, 1, 1);
          xjjroot::sethempty(href[p][i], 0, 0.2); href[p][i]->GetXaxis()->SetNdivisions(505);
          xjjroot::setthgrstyle(href[p][i], color, 20, 0.9, color, 1, 1);
          xjjroot::sethempty(hgen_norm[p][i], 0, 0.2); hgen_norm[p][i]->GetXaxis()->SetNdivisions(505);
          xjjroot::setthgrstyle(hgen_norm[p][i], color, 20, 0.9, color, 1, 1);
          xjjroot::sethempty(href_norm[p][i], 0, 0.2); href_norm[p][i]->GetXaxis()->SetNdivisions(505);
          xjjroot::setthgrstyle(href_norm[p][i], color, 20, 0.9, color, 1, 1);
          if(!p) leg->AddEntry(hgen[p][i], Form("%.0f < p_{T}^{gen jet} < %.0f GeV", vbpt[i], vbpt[i+1]), "l");
        }
    }
  xjjroot::setgstyle(1);
  gStyle->SetLineScalePS(2);
  xjjroot::mypdf* fpdf = new xjjroot::mypdf("plots/"+outsubdir+"/cptdif_"+var+".pdf", "c", 600*vbDpt.n(), 600);

  // linear
  fpdf->getc()->SetLogy(0);
  // hgen
  fpdf->prepare();
  for(int p=0; p<vbDpt.n(); p++)
    {
      TPad* pad = new TPad(Form("p%s%d", xjjc::currenttime().c_str(), p), "", p*1.0/vbDpt.n(), 0., (p+1)*1.0/vbDpt.n(), 1.); pad->Draw(); pad->cd();
      ptdif::drawthese(fpdf, hgen[p], leg, Form("%s < p_{T}^{D} < %s GeV", xjjc::number_remove_zero(vbDpt[p]).c_str(), xjjc::number_remove_zero(vbDpt[p+1]).c_str()), thi, false);
      fpdf->getc()->cd();
    }
  fpdf->write();
  // href
  fpdf->prepare();
  for(int p=0; p<vbDpt.n(); p++)
    {
      TPad* pad = new TPad(Form("p%s%d", xjjc::currenttime().c_str(), p), "", p*1.0/vbDpt.n(), 0., (p+1)*1.0/vbDpt.n(), 1.); pad->Draw(); pad->cd();
      ptdif::drawthese(fpdf, href[p], leg, Form("%s < p_{T}^{D} < %s GeV", xjjc::number_remove_zero(vbDpt[p]).c_str(), xjjc::number_remove_zero(vbDpt[p+1]).c_str()), thi, false, "p_{T}^{reco jet} > 80 GeV/c");
      fpdf->getc()->cd();
    }
  fpdf->write();
  // log
  // hgen
  fpdf->prepare();
  for(int p=0; p<vbDpt.n(); p++)
    {
      TPad* pad = new TPad(Form("p%s%d", xjjc::currenttime().c_str(), p), "", p*1.0/vbDpt.n(), 0., (p+1)*1.0/vbDpt.n(), 1.); pad->Draw(); pad->cd();
      pad->SetLogy(1);
      ptdif::drawthese(fpdf, hgen[p], leg, Form("%s < p_{T}^{D} < %s GeV", xjjc::number_remove_zero(vbDpt[p]).c_str(), xjjc::number_remove_zero(vbDpt[p+1]).c_str()), thi, true);
      fpdf->getc()->cd();
    }
  fpdf->write();
  // href
  fpdf->prepare();
  for(int p=0; p<vbDpt.n(); p++)
    {
      TPad* pad = new TPad(Form("p%s%d", xjjc::currenttime().c_str(), p), "", p*1.0/vbDpt.n(), 0., (p+1)*1.0/vbDpt.n(), 1.); pad->Draw(); pad->cd();
      pad->SetLogy(1);
      ptdif::drawthese(fpdf, href[p], leg, Form("%s < p_{T}^{D} < %s GeV", xjjc::number_remove_zero(vbDpt[p]).c_str(), xjjc::number_remove_zero(vbDpt[p+1]).c_str()), thi, true, "p_{T}^{reco jet} > 80 GeV/c");
      fpdf->getc()->cd();
    }
  fpdf->write();
  // linear
  fpdf->getc()->SetLogy(0);
  // hgen_norm
  fpdf->prepare();
  for(int p=0; p<vbDpt.n(); p++)
    {
      TPad* pad = new TPad(Form("p%s%d", xjjc::currenttime().c_str(), p), "", p*1.0/vbDpt.n(), 0., (p+1)*1.0/vbDpt.n(), 1.); pad->Draw(); pad->cd();
      ptdif::drawthese(fpdf, hgen_norm[p], leg, Form("%s < p_{T}^{D} < %s GeV", xjjc::number_remove_zero(vbDpt[p]).c_str(), xjjc::number_remove_zero(vbDpt[p+1]).c_str()), thi, false);
      fpdf->getc()->cd();
    }
  fpdf->write();
  // href_norm
  fpdf->prepare();
  for(int p=0; p<vbDpt.n(); p++)
    {
      TPad* pad = new TPad(Form("p%s%d", xjjc::currenttime().c_str(), p), "", p*1.0/vbDpt.n(), 0., (p+1)*1.0/vbDpt.n(), 1.); pad->Draw(); pad->cd();
      ptdif::drawthese(fpdf, href_norm[p], leg, Form("%s < p_{T}^{D} < %s GeV", xjjc::number_remove_zero(vbDpt[p]).c_str(), xjjc::number_remove_zero(vbDpt[p+1]).c_str()), thi, false, "p_{T}^{reco jet} > 80 GeV/c");
      fpdf->getc()->cd();
    }
  fpdf->write();
  // log
  fpdf->getc()->SetLogy(1);
  // hgen_norm
  fpdf->prepare();
  for(int p=0; p<vbDpt.n(); p++)
    {
      TPad* pad = new TPad(Form("p%s%d", xjjc::currenttime().c_str(), p), "", p*1.0/vbDpt.n(), 0., (p+1)*1.0/vbDpt.n(), 1.); pad->Draw(); pad->cd();
      pad->SetLogy(1);
      ptdif::drawthese(fpdf, hgen_norm[p], leg, Form("%s < p_{T}^{D} < %s GeV", xjjc::number_remove_zero(vbDpt[p]).c_str(), xjjc::number_remove_zero(vbDpt[p+1]).c_str()), thi, true);
      fpdf->getc()->cd();
    }
  fpdf->write();
  // href_norm
  fpdf->prepare();
  for(int p=0; p<vbDpt.n(); p++)
    {
      TPad* pad = new TPad(Form("p%s%d", xjjc::currenttime().c_str(), p), "", p*1.0/vbDpt.n(), 0., (p+1)*1.0/vbDpt.n(), 1.); pad->Draw(); pad->cd();
      pad->SetLogy(1);
      ptdif::drawthese(fpdf, href_norm[p], leg, Form("%s < p_{T}^{D} < %s GeV", xjjc::number_remove_zero(vbDpt[p]).c_str(), xjjc::number_remove_zero(vbDpt[p+1]).c_str()), thi, true, "p_{T}^{reco jet} > 80 GeV/c");
      fpdf->getc()->cd();
    }
  fpdf->write();


  fpdf->close();
  return 0;
}

int main(int argc, char* argv[])
{
  if(argc == 4)
    return ptdif_drawhist(argv[1], argv[2], argv[3]);

  return 1;
}

void ptdif::drawthese(xjjroot::mypdf* pdf, std::vector<TH1D*> vh, TLegend* leg, 
                      std::string tex, std::string collision, bool islogy, std::string comment)
{
  double ymax = 0, ymin = 1.e+10;
  for(auto& h : vh)
    {
      if(xjjana::gethmaximum(h) > ymax) 
        ymax = xjjana::gethmaximum(h);
      if(xjjana::gethminimum(h) < ymin) 
        ymin = xjjana::gethminimum(h);
    }
  std::vector<std::string> vtex = {"|#eta^{jet}| < 1.6", "|#eta^{D}| < 1.2", tex.c_str(), comment};
  int iy;

  // linear
  if(islogy)
    for(auto& h : vh) 
      xjjana::sethabsminmax(h, (ymin>0?ymin:0.01)*0.5, ymax*10.);
  else
    for(auto& h : vh) 
      xjjana::sethabsminmax(h, 0, ymax*1.3);
  vh[0]->Draw("AXIS");
  for(auto& h : vh) 
    {
      auto gr = xjjana::histtograph(h, Form("gr_%s", h->GetName()));
      gr->Draw("lx same");
    }
  leg->Draw();
  iy = 0; for(auto& t : vtex)
            xjjroot::drawtex(0.25, 0.85-0.035*(++iy), t.c_str(), 0.032);
  xjjroot::drawCMS("Simulation", Form("%s #sqrt{s_{NN}} = 5.02 TeV", collision.c_str()));
}
