#include <TFile.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TH1F.h>
#include <iostream>
#include <iomanip>

#include "param.h"

#include "xjjcuti.h"
#include "xjjrootuti.h"

void seth(TH1F* h);
int mcana_drawhist(std::string inputname, std::string outsubdir, int cutopt)
{
  TFile* inf = TFile::Open(inputname.c_str());
  phoD::param pa(inf);
  float purity = pa.ishi()?phoD::purity_aa_:phoD::purity_pp_;
 
  std::map<std::string, TH1F*> hdphi;
  std::map<std::string, float> nphoton;

  std::vector<std::string> its = {"mc", "gen", "gen_Kpi"};
  for(auto& tt : its)
    {
      hdphi[tt] = (TH1F*)inf->Get(Form("hdphi_%s", tt.c_str()));
      hdphi[tt]->GetXaxis()->SetTitle("#Delta#phi^{#gammaD} / #pi");
      nphoton[tt] = atof(hdphi[tt]->GetTitle());

      hdphi[tt+"_selfnorm"] = (TH1F*)hdphi[tt]->Clone(Form("hdphi_%s_selfnorm", tt.c_str()));
      hdphi[tt+"_selfnorm"]->Scale(1./(hdphi[tt]->Integral()*M_PI), "width");
      hdphi[tt+"_selfnorm"]->GetYaxis()->SetTitle("#frac{1}{N^{#gammaD}} #frac{dN^{#gammaD}}{d#phi}");
      seth(hdphi[tt+"_selfnorm"]);

      hdphi[tt]->Scale(1./(nphoton[tt]*M_PI), "width");
      hdphi[tt]->GetYaxis()->SetTitle("#frac{1}{N^{#gamma}} #frac{dN^{#gammaD}}{d#phi}");
      seth(hdphi[tt]);
    }

  xjjroot::setthgrstyle(hdphi["mc"], xjjroot::mycolor_middle["red"], 20, 1, xjjroot::mycolor_middle["red"], 1, 1);
  xjjroot::setthgrstyle(hdphi["mc_selfnorm"], xjjroot::mycolor_middle["red"], 20, 1, xjjroot::mycolor_middle["red"], 1, 1);

  xjjroot::setthgrstyle(hdphi["gen"], xjjroot::mycolor_satmiddle["azure"], 20, 1, xjjroot::mycolor_satmiddle["azure"], 1, 1);
  xjjroot::setthgrstyle(hdphi["gen_selfnorm"], xjjroot::mycolor_satmiddle["azure"], 20, 1, xjjroot::mycolor_satmiddle["azure"], 1, 1);
  xjjroot::setthgrstyle(hdphi["gen_Kpi"], xjjroot::mycolor_satmiddle["azure"], 24, 1, xjjroot::mycolor_satmiddle["azure"], 1, 1);
  xjjroot::setthgrstyle(hdphi["gen_Kpi_selfnorm"], xjjroot::mycolor_satmiddle["azure"], 24, 1, xjjroot::mycolor_satmiddle["azure"], 1, 1);

  std::string outputname = "rootfiles/" + outsubdir + "_" + pa.tag() + "/drawhist.root";
  xjjroot::mkdir(outputname);
  TFile* outf = new TFile(outputname.c_str(), "recreate");
  for(auto& hh : hdphi) xjjroot::writehist(hh.second);
  pa.write();
  outf->Close();

  TGaxis::SetExponentOffset(-0.1, 0, "y");
  xjjroot::setgstyle(1);
  TCanvas* c;
  TLegend* leg;
  std::string syst = (cutopt==4?"P + H":"PYTHIA8");

  leg = new TLegend(0.22, 0.65-0.04, 0.50, 0.65);
  xjjroot::setleg(leg, 0.035);
  leg->AddEntry(hdphi["mc"], "Reco");
  c = new TCanvas("c", "", 600, 600);
  hdphi["mc"]->Draw("pe");
  leg->Draw();
  pa.drawtex(0.23, 0.85, 0.035, "cent");
  xjjroot::drawtex(0.90, 0.845, syst.c_str(), 0.035, 33, 62);
  if(pa.ishi()) xjjroot::drawtex(0.90, 0.845-0.04, "HYDJET reco", 0.035, 33, 62);
  xjjroot::drawCMSleft("Simulation");
  xjjroot::drawCMSright(Form("%s #sqrt{s_{NN}} = 5.02 TeV", pa.tag("ishi").c_str()));
  xjjroot::drawcomment(outsubdir);
  std::string output = "plots/" + outsubdir + "_" + pa.tag() + "/chdphi_mc.pdf";
  xjjroot::mkdir(output);
  c->SaveAs(output.c_str());
  delete c;

  leg = new TLegend(0.22, 0.65-1*0.04, 0.50, 0.65);
  xjjroot::setleg(leg, 0.035);
  leg->AddEntry(hdphi["gen"], "Gen");
  c = new TCanvas("c", "", 600, 600);
  hdphi["gen"]->Draw("pe");
  leg->Draw();
  pa.drawtex(0.23, 0.85, 0.035, "cent");
  xjjroot::drawtex(0.90, 0.845, syst.c_str(), 0.035, 33, 62);
  xjjroot::drawCMSleft("Simulation");
  xjjroot::drawCMSright(Form("%s #sqrt{s_{NN}} = 5.02 TeV", pa.tag("ishi").c_str()));
  xjjroot::drawcomment(outsubdir);
  output = "plots/" + outsubdir + "_" + pa.tag() + "/chdphi_gen.pdf";
  xjjroot::mkdir(output);
  c->SaveAs(output.c_str());
  delete c;

  leg = new TLegend(0.22, 0.65-2*0.04, 0.50, 0.65);
  xjjroot::setleg(leg, 0.035);
  leg->AddEntry(hdphi["gen"], "Gen");
  leg->AddEntry(hdphi["gen_Kpi"], "Gen D^{0}#rightarrowK#pi");
  c = new TCanvas("c", "", 600, 600);
  hdphi["gen"]->Draw("pe");
  hdphi["gen_Kpi"]->Draw("pe same");
  leg->Draw();
  pa.drawtex(0.23, 0.85, 0.035, "cent");
  xjjroot::drawtex(0.90, 0.845, syst.c_str(), 0.035, 33, 62);
  xjjroot::drawCMSleft("Simulation");
  xjjroot::drawCMSright(Form("%s #sqrt{s_{NN}} = 5.02 TeV", pa.tag("ishi").c_str()));
  xjjroot::drawcomment(outsubdir);
  output = "plots/" + outsubdir + "_" + pa.tag() + "/chdphi_gen_Kpi.pdf";
  xjjroot::mkdir(output);
  c->SaveAs(output.c_str());
  delete c;

  leg = new TLegend(0.22, 0.65-3*0.04, 0.50, 0.65);
  xjjroot::setleg(leg, 0.035);
  leg->AddEntry(hdphi["gen_selfnorm"], "Gen");
  leg->AddEntry(hdphi["gen_Kpi_selfnorm"], "Gen D^{0}#rightarrowK#pi");
  leg->AddEntry(hdphi["mc_selfnorm"], "Reco");
  c = new TCanvas("c", "", 600, 600);
  hdphi["gen_selfnorm"]->Draw("pe");
  hdphi["gen_Kpi_selfnorm"]->Draw("pe same");
  hdphi["mc_selfnorm"]->Draw("pe same");
  leg->Draw();
  pa.drawtex(0.23, 0.85, 0.035, "cent");
  xjjroot::drawtex(0.90, 0.845, syst.c_str(), 0.035, 33, 62);
  if(pa.ishi()) xjjroot::drawtex(0.90, 0.845-0.04, "HYDJET reco", 0.035, 33, 62);
  xjjroot::drawCMSleft("Simulation");
  xjjroot::drawCMSright(Form("%s #sqrt{s_{NN}} = 5.02 TeV", pa.tag("ishi").c_str()));
  xjjroot::drawcomment(outsubdir);
  output = "plots/" + outsubdir + "_" + pa.tag() + "/chdphi_genreco.pdf";
  xjjroot::mkdir(output);
  c->SaveAs(output.c_str());

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc==4)
    return mcana_drawhist(argv[1], argv[2], atoi(argv[3]));
  return 1;
}

void seth(TH1F* h)
{
  h->SetMinimum(0);
  h->SetMaximum(h->GetMaximum()*1.4);
  h->GetXaxis()->SetNdivisions(-505);
  h->GetYaxis()->SetMaxDigits(1);
  xjjroot::sethempty(h, 0, 0.1);
}
