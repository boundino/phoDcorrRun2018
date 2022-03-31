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

namespace mcana_
{
  void seth(TH1F* h, bool forcemaxdigits=false);
  void makecanvas(xjjroot::mypdf* fpdf, Djet::param& pa, TLegend* leg);
}

int mcana_drawhist(std::string inputname, std::string outsubdir)
{
  TFile* inf = TFile::Open(inputname.c_str());
  Djet::param pa(inf);
  std::string outputdir = outsubdir + "_" + pa.tag();

  const int NTYPE = 6;
  std::map<std::string, TH1F*> hd[NTYPE], hg[NTYPE];
  std::map<std::string, TGraphErrors*> grd[NTYPE], grg[NTYPE];
  // Color_t cc[NTYPE] = {kRed, kBlue, kGreen+2, kBlack, kGray, kRed};
  Color_t cc[NTYPE] = {kBlack, kGreen+2, kBlue, kRed, kRed, kGray};
  Style_t ss[NTYPE] = {21, 20, 20, 20, 25, 20};
  // std::string tleg[NTYPE] = {"Non-P D, Non-P jet", "Non-P D, PYTHIA jet", "PYTHIA D, Non-P jet", "PYTHIA D, PYTHIA jet", "All", "Same Hydjet event D-jet"};
  std::string tleg[NTYPE] = {"PYTHIA D, PYTHIA jet", "PYTHIA D, Non-P jet", "Non-P D, PYTHIA jet", "Non-P D, Non-P jet (Different)", "Non-P D, Non-P jet (Same Hydjet)", "All"};
  for(auto& v : Djet::var)
    {
      for(int k=0; k<NTYPE; k++)
        {
          hd[k][v] = xjjroot::gethist<TH1F>(inf, Form("h%s_%d", v.c_str(), k));
          hg[k][v] = xjjroot::gethist<TH1F>(inf, Form("h%s_gen_%d", v.c_str(), k));
        }
      xjjana::bins<double> vb(xjjana::gethXaxis(hd[0][v]));
      for(int k=0; k<NTYPE; k++)
        {
          for(int l=0; l<vb.n(); l++)
            {
              float bin_width = vb.width(l); // dphi
              if(v=="dr") bin_width = vb.area(l);
              hd[k][v]->SetBinContent(l+1, hd[k][v]->GetBinContent(l+1)/bin_width);
              hd[k][v]->SetBinError(l+1, hd[k][v]->GetBinError(l+1)/bin_width);
              hg[k][v]->SetBinContent(l+1, hg[k][v]->GetBinContent(l+1)/bin_width);
              hg[k][v]->SetBinError(l+1, hg[k][v]->GetBinError(l+1)/bin_width);
            }
          grd[k][v] = xjjana::shifthistcenter(hd[k][v], Form("grd_%s", hd[k][v]->GetName()), 0);
          mcana_::seth(hd[k][v]);
          xjjroot::setthgrstyle(grd[k][v], cc[k], ss[k], 1, cc[k], 1, 2);
          grg[k][v] = xjjana::shifthistcenter(hg[k][v], Form("grg_%s", hg[k][v]->GetName()), 0);
          mcana_::seth(hg[k][v]);
          xjjroot::setthgrstyle(grg[k][v], cc[k], ss[k], 1, cc[k], 1, 2);
        }
    }

  auto leg = new TLegend(0.50, 0.85-6*0.04, 0.75, 0.85);
  xjjroot::setleg(leg, 0.033);
  for(int k=0; k<NTYPE; k++)
    leg->AddEntry(grd[k]["dphi"], tleg[k].c_str(), "pl");

  TGaxis::SetExponentOffset(-0.1, 0, "y");
  xjjroot::setgstyle(1);
  xjjroot::mypdf* fpdf = new xjjroot::mypdf("plots/"+outputdir+"/cgen.pdf", "c", 600, 600);
  for(auto& v : Djet::var)
    {
      fpdf->prepare();
      hd[5][v]->Draw("AXIS");
      for(int k=0; k<NTYPE; k++)
        grd[k][v]->Draw("pe same");
      mcana_::makecanvas(fpdf, pa, leg);

      fpdf->prepare();
      hg[5][v]->Draw("AXIS");
      for(int k=0; k<NTYPE; k++)
        grg[k][v]->Draw("pe same");
      mcana_::makecanvas(fpdf, pa, leg);
    }
  fpdf->close();
  return 0;
}

int main(int argc, char* argv[])
{
  if(argc==3)
    return mcana_drawhist(argv[1], argv[2]);
  return 1;
}

void mcana_::seth(TH1F* h, bool forcemaxdigits)
{
  h->SetMinimum(0);
  h->SetMaximum(h->GetMaximum()*1.6);
  h->GetXaxis()->SetNdivisions(505);
  if(forcemaxdigits) h->GetYaxis()->SetMaxDigits(1);
  xjjroot::sethempty(h, 0, 0.1);
}


void mcana_::makecanvas(xjjroot::mypdf* fpdf, Djet::param& pa, TLegend* leg)
{
  pa.drawtex(0.23, 0.85, 0.035, "cent");
  leg->Draw();
  xjjroot::drawCMSleft("Simulation");
  xjjroot::drawCMSright("#sqrt{s_{NN}} = 5.02 TeV");
  // xjjroot::drawtex(0.92, 0.80, (xjjc::str_contains(fpdf->getfilename(), "embed")?"PYTHIA + HEDJET":"PYTHIA 8"), 0.035, 33, 62);
  auto note = xjjc::str_eraseall(fpdf->getfilename(), "plots/");
  note = xjjc::str_eraseall(note, "cgen.pdf");
  xjjroot::drawcomment(note, "lb");
  fpdf->write();
}

