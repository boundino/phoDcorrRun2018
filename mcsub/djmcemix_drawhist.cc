#include <TFile.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TH1F.h>
#include <iostream>
#include <iomanip>

#include "para.h"
#include "bins.h"

#include "xjjcuti.h"
#include "xjjrootuti.h"
#include "xjjanauti.h"
#include "xjjmypdf.h"

#include "mcana.h"

TH1F* get_and_scale(TFile* inf, std::string name, std::string newname, std::string v);
int mcana_drawhist(std::string inputname_incl, std::string inputname_emix, std::string inputname_truth, std::string outsubdir)
{
  auto inf_incl = TFile::Open(inputname_incl.c_str());
  Djet::param pa(inf_incl);
  std::string outputdir = outsubdir + "_" + pa.tag();
  auto inf_emix = TFile::Open(inputname_emix.c_str());
  auto inf_truth = TFile::Open(inputname_truth.c_str());

  std::map<std::string, TH1F*> hd_incl, hd_emix, hd_sub,
    hd_P_P, hd_PH_PH, hd_P_H, hd_H_P, hd_H_H_diff, hd_H_H_same, hd_H_H, hd_H_PH;
  for(auto& v : Djet::var)
    {
      hd_incl[v] = get_and_scale(inf_incl, Form("h%s", v.c_str()), Form("h%s_incl", v.c_str()), v);
      xjjroot::setthgrstyle(hd_incl[v], kBlack, 21, 1, kBlack, 1, 2);
      hd_emix[v] = get_and_scale(inf_emix, Form("h%s", v.c_str()), Form("h%s_emix", v.c_str()), v);
      xjjroot::setthgrstyle(hd_emix[v], kBlack, 25, 1, kBlack, 1, 2);

      hd_P_P[v] = get_and_scale(inf_truth, Form("h%s_0", v.c_str()), Form("h%s_P_P", v.c_str()), v);
      xjjroot::setthgrstyle(hd_P_P[v], kBlack, 21, 1, kBlack, 1, 2);
      hd_PH_PH[v] = get_and_scale(inf_truth, Form("h%s_5", v.c_str()), Form("h%s_PH_PH", v.c_str()), v);
      xjjroot::setthgrstyle(hd_PH_PH[v], kGray, 20, 1, kGray, 1, 2);
      hd_P_H[v] = get_and_scale(inf_truth, Form("h%s_1", v.c_str()), Form("h%s_P_H", v.c_str()), v);
      hd_H_P[v] = get_and_scale(inf_truth, Form("h%s_2", v.c_str()), Form("h%s_H_P", v.c_str()), v);
      hd_H_H_diff[v] = get_and_scale(inf_truth, Form("h%s_3", v.c_str()), Form("h%s_H_H_diff", v.c_str()), v);
      hd_H_H_same[v] = get_and_scale(inf_truth, Form("h%s_4", v.c_str()), Form("h%s_H_H_same", v.c_str()), v);

      hd_H_H[v] = (TH1F*)hd_H_H_diff[v]->Clone(Form("h%s_H_H", v.c_str()));
      hd_H_H[v]->Add(hd_H_H_same[v]);
      hd_H_PH[v] = (TH1F*)hd_H_P[v]->Clone(Form("h%s_H_PH", v.c_str()));
      hd_H_PH[v]->Add(hd_H_H[v]);
      xjjroot::setthgrstyle(hd_H_PH[v], kBlue, 20, 1, kBlue, 1, 2);

      hd_sub[v] = (TH1F*)hd_incl[v]->Clone(Form("h%s_sub", v.c_str()));
      hd_sub[v]->Add(hd_emix[v], -1);
      xjjroot::setthgrstyle(hd_sub[v], kOrange+3, 20, 1, kOrange+3, 1, 2);
    }

  auto leg = new TLegend(0.50, 0.85-6*0.04, 0.75, 0.85);
  xjjroot::setleg(leg, 0.033);

  TGaxis::SetExponentOffset(-0.1, 0, "y");
  xjjroot::setgstyle(1);
  xjjroot::mypdf* fpdf = new xjjroot::mypdf("plots/"+outputdir+"/cclosure.pdf", "c", 600, 600);
  for(auto& v : Djet::var)
    {
      fpdf->prepare();
      hd_incl[v]->Draw("pe");
      hd_PH_PH[v]->Draw("pe same");
      mcana_::makecanvas(fpdf, pa, leg);

      fpdf->prepare();
      hd_emix[v]->Draw("pe");
      hd_incl[v]->Draw("pe same");
      hd_H_PH[v]->Draw("pe same");
      mcana_::makecanvas(fpdf, pa, leg);
    }
  fpdf->close();
  return 0;

}

int main(int argc, char* argv[])
{
  if(argc == 5)
    return mcana_drawhist(argv[1], argv[2], argv[3], argv[4]);
  return 1;
}

TH1F* get_and_scale(TFile* inf, std::string name, std::string newname, std::string v)
{
  auto h = xjjroot::gethist<TH1F>(inf, name.c_str());
  xjjana::bins<double> vb(xjjana::gethXaxis(h));
  h->SetName(newname.c_str());
  for(int l=0; l<vb.n(); l++)
    {
      float bin_width = vb.width(l); // dphi
      if(v=="dr") bin_width = vb.area(l);
      h->SetBinContent(l+1, h->GetBinContent(l+1)/bin_width);
      h->SetBinError(l+1, h->GetBinError(l+1)/bin_width);
    }
  mcana_::seth(h);
  return h;
}
