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

  std::map<std::string, TH1F*> hd;
  std::map<std::string, TGraphErrors*> gr;
  for(auto& v : Djet::var)
    {
      hd[v] = xjjroot::gethist<TH1F>(inf, Form("h%s_gen", v.c_str()));
      xjjana::bins<double> vb(xjjana::gethXaxis(hd[v]));
      float njet = atof(hd[v]->GetTitle());
      for(int k=0; k<vb.n(); k++)
        {
          float bin_width = vb.width(k) * M_PI; // dphi
          if(v=="dr") bin_width = vb.area(k);
          hd[v]->SetBinContent(k+1, hd[v]->GetBinContent(k+1)/njet/bin_width);
          hd[v]->SetBinError(k+1, hd[v]->GetBinError(k+1)/njet/bin_width);
          // std::cout<<k<<" "<<bin_width<<" "<<hd[v]->GetBinContent(k+1)<<" "<<hd[v]->GetBinError(k+1)<<std::endl;
        }
      hd[v]->GetYaxis()->SetTitle(Form("#frac{1}{N^{jet}} %s", hd[v]->GetYaxis()->GetTitle()));
      gr[v] = xjjana::shifthistcenter(hd[v], "gr"+v, 0);
      mcana_::seth(hd[v]);
      xjjroot::setthgrstyle(gr[v], kBlack, 20, 1, kBlack, 1, 2);
    }

  std::string outputname = "rootfiles/" + outputdir + "/drawhist.root";
  xjjroot::mkdir(outputname);
  TFile* outf = new TFile(outputname.c_str(), "recreate");
  for(auto& hh : hd) xjjroot::writehist(hh.second);
  pa.write();
  outf->Close();

  std::map<std::string, TLegend*> leg;
  for(auto& v : Djet::var)
    {
      leg["gen_"+v] = new TLegend(0.22, 0.65-0.04*1, 0.50, 0.65);
      leg["gen_"+v]->AddEntry(gr[v], "Gen", "l");
    }
  for(auto& ll : leg) xjjroot::setleg(ll.second, 0.035);

  TGaxis::SetExponentOffset(-0.1, 0, "y");
  xjjroot::setgstyle(1);
  xjjroot::mypdf* fpdf = new xjjroot::mypdf("plots/"+outputdir+"/cgen.pdf", "c", 600, 600);
  for(auto& v : Djet::var)
    {
      fpdf->prepare();
      hd[v]->Draw("AXIS");
      gr[v]->Draw("lX0 same");
      mcana_::makecanvas(fpdf, pa, leg["gen_"+v]);
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
  h->GetXaxis()->SetNdivisions(-505);
  if(forcemaxdigits) h->GetYaxis()->SetMaxDigits(1);
  xjjroot::sethempty(h, 0, 0.1);
}


void mcana_::makecanvas(xjjroot::mypdf* fpdf, Djet::param& pa, TLegend* leg)
{
  pa.drawtex(0.23, 0.85, 0.035, "cent");
  leg->Draw();
  xjjroot::drawCMSleft("Simulation");
  xjjroot::drawCMSright("#sqrt{s_{NN}} = 5.02 TeV");
  xjjroot::drawtex(0.92, 0.80, (xjjc::str_contains(fpdf->getfilename(), "embed")?"PYTHIA + HEDJET":"PYTHIA 8"), 0.035, 33, 62);
  auto note = xjjc::str_eraseall(fpdf->getfilename(), "plots/");
  note = xjjc::str_eraseall(note, "cgen.pdf");
  xjjroot::drawcomment(note, "lb");
  fpdf->write();
}

