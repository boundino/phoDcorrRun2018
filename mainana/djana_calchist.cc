#include <TFile.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <iostream>
#include <iomanip>

#include "para.h"

#include "xjjcuti.h"
#include "xjjrootuti.h"
#include "xjjmypdf.h"
#include "pdg.h"

namespace djana_
{
  void seth(TH1F* h, std::string ytitle="", bool forcemaxdigits=true, bool setminmax=true);
  void makecanvas(xjjroot::mypdf* f, Djet::param& pa, TLegend* leg, std::string comment="");
}

int djana_calchist(std::string inputname, std::string outsubdir)
{
  TFile* inf = TFile::Open(inputname.c_str());
  Djet::param pa(inf);
  std::string outputdir = outsubdir + "_" + pa.tag();

  std::vector<std::string> type = {"fitweigh", "unweight", "effcorr"};
  std::map<std::string, std::string> tags;
  tags["fitweigh"] = "D eff weight fit";
  tags["unweight"] = "D eff uncorr";
  tags["effcorr"] = "D eff corr";
  std::map<std::string, TH1F*> hd, heff;
  for(auto& v : Djet::var) // dphi, dr
    {
      for(auto& t : type)
        {
          hd[v+"_"+t] = xjjroot::gethist<TH1F>(inf, Form("h%s_%s", v.c_str(), t.c_str()));
          float njet = atof(hd[v+"_"+t]->GetTitle());
          hd[v+"_"+t]->Scale(1./njet);
          hd[v+"_"+t]->GetYaxis()->SetTitle(Form("#frac{1}{N^{jet}} %s", hd[v+"_"+t]->GetYaxis()->GetTitle()));

          hd[v+"_"+t+"_sbr"] = (TH1F*)hd[v+"_"+t]->Clone(Form("h%s_sbr_%s", v.c_str(), t.c_str()));
          hd[v+"_"+t+"_sbr"]->Scale(1./pdg::BR_DZERO_KPI);
        }
      heff[v] = xjjroot::gethist<TH1F>(inf, Form("heff_%s", v.c_str()));
      heff[v]->GetYaxis()->SetTitle("< 1 / #alpha #times #varepsilon >");
      djana_::seth(heff[v], "", false);
      xjjroot::setthgrstyle(heff[v], kBlack, 20, 1, kBlack, 1, 2);
    }

  std::string output = "rootfiles/" + outputdir + "/calchist.root";
  xjjroot::mkdir(output);
  TFile* outf = new TFile(output.c_str(), "recreate");
  for(auto& hh : hd) xjjroot::writehist(hh.second);
  pa.write();
  outf->Close();

  for(auto& hh : hd)
    {
      djana_::seth(hh.second);
      xjjroot::setthgrstyle(hh.second, kBlack, 20, 1, kBlack, 1, 2);
    }

  std::map<std::string, TLegend*> leg;
  leg["sbr"] = new TLegend(0.22, 0.65-0.04, 0.50, 0.65);
  leg["sbr"]->AddEntry(hd["dphi_effcorr_sbr"], "Raw / BR", "pl");
  leg["raw"] = new TLegend(0.22, 0.65-0.04, 0.50, 0.65);
  leg["raw"]->AddEntry(hd["dphi_effcorr"], "Raw", "pl");
  leg["eff"] = new TLegend(0.22, 0.65-0.04, 0.50, 0.65);
  leg["eff"]->AddEntry(heff["dphi"], "Raw", "pl");
  for(auto& ll : leg) xjjroot::setleg(ll.second, 0.035);

  TGaxis::SetExponentOffset(-0.1, 0, "y");
  xjjroot::setgstyle(1);

  for(auto& v : Djet::var)
    {
      xjjroot::mypdf* fpdf = new xjjroot::mypdf("plots/"+outputdir+"/ch"+v+".pdf", "c", 600, 600);
      fpdf->prepare();
      heff[v]->Draw("pe");
      djana_::makecanvas(fpdf, pa, leg["eff"], "");

      for(auto& t :type)
        {
          fpdf->prepare();
          hd[v+"_"+t]->Draw("pe");
          djana_::makecanvas(fpdf, pa, leg["raw"], tags[t]);

          fpdf->prepare();
          hd[v+"_"+t+"_sbr"]->Draw("pe");
          djana_::makecanvas(fpdf, pa, leg["sbr"], tags[t]);
        }
      fpdf->close();
    }

  return 0;
}

void djana_::seth(TH1F* h, std::string ytitle, bool forcemaxdigits, bool setminmax)
{
  xjjroot::sethempty(h, 0, 0.1);
  if(setminmax)
    {
      h->SetMinimum(0);
      h->SetMaximum(h->GetMaximum()*1.7);
    }
  h->GetXaxis()->SetNdivisions(505);
  if(forcemaxdigits) h->GetYaxis()->SetMaxDigits(1);
  if(ytitle != "") h->GetYaxis()->SetTitle(ytitle.c_str());
}

int main(int argc, char* argv[])
{
  if(argc==3)
    return djana_calchist(argv[1], argv[2]);
  return 1;
}

void djana_::makecanvas(xjjroot::mypdf* f, Djet::param& pa, TLegend* leg, std::string comment)
{
  pa.drawtex(0.23, 0.85, 0.035);
  leg->Draw();
  xjjroot::drawCMSleft();
  xjjroot::drawCMSright(Form("%s #sqrt{s_{NN}} = 5.02 TeV", pa.tag("ishi").c_str()));
  xjjroot::drawtex(0.92, 0.80, comment.c_str(), 0.035, 33, 62);
  f->write();
}
