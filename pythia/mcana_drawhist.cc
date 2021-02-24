#include <TFile.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TH1F.h>
#include <iostream>
#include <iomanip>

#include "param.h"
#include "mcana.h"

#include "xjjcuti.h"
#include "xjjrootuti.h"

namespace mcana_drawhist_
{
  void seth(TH1F* h, bool forcemaxdigits=true);
  std::string outputdir;
  void makecanvas(TCanvas* c, phoD::param& pa, TLegend* leg, std::string name, std::string comment="");
}

int mcana_drawhist(std::string inputname, std::string outsubdir, int isembed)
{
  TFile* inf = TFile::Open(inputname.c_str());
  phoD::param pa(inf);
  mcana_drawhist_::outputdir = outsubdir + "_" + pa.tag();
 
  for(auto& tt : mcana_::types)
    {
      mcana_::hdphi[tt] = (TH1F*)inf->Get(Form("hdphi_%s", tt.c_str()));
      mcana_::hdphi[tt]->GetXaxis()->SetTitle("#Delta#phi^{#gammaD} / #pi");
      float nphoton = atof(mcana_::hdphi[tt]->GetTitle());

      mcana_::hdphi[tt+"_selfnorm"] = (TH1F*)mcana_::hdphi[tt]->Clone(Form("hdphi_%s_selfnorm", tt.c_str()));
      mcana_::hdphi[tt+"_selfnorm"]->Scale(1./(mcana_::hdphi[tt]->Integral()*M_PI), "width");
      mcana_::hdphi[tt+"_selfnorm"]->GetYaxis()->SetTitle("#frac{1}{N^{#gammaD}} #frac{dN^{#gammaD}}{d#phi}");
      mcana_drawhist_::seth(mcana_::hdphi[tt+"_selfnorm"]);

      mcana_::hdphi[tt]->Scale(1./(nphoton*M_PI), "width");
      mcana_::hdphi[tt]->GetYaxis()->SetTitle("#frac{1}{N^{#gamma}} #frac{dN^{#gammaD}}{d#phi}");
      mcana_drawhist_::seth(mcana_::hdphi[tt]);

      int color = xjjc::str_contains(tt, "mc_")?xjjroot::mycolor_middle["red"]:xjjroot::mycolor_satmiddle["azure"];
      Style_t mstyle = xjjc::str_contains(tt, "R4")?20:24;
      Style_t lstyle = xjjc::str_contains(tt, "R4")?1:7;
      xjjroot::setthgrstyle(mcana_::hdphi[tt], color, mstyle, 1, color, lstyle, 1);
      xjjroot::setthgrstyle(mcana_::hdphi[tt+"_selfnorm"], color, mstyle, 1, color, lstyle, 1);
    }

  std::string outputname = "rootfiles/" + mcana_drawhist_::outputdir + "/drawhist.root";
  xjjroot::mkdir(outputname);
  TFile* outf = new TFile(outputname.c_str(), "recreate");
  for(auto& hh : mcana_::hdphi) xjjroot::writehist(hh.second);
  pa.write();
  outf->Close();

  std::map<std::string, TLegend*> leg;
  leg["mc"] = new TLegend(0.22, 0.65-0.04*3, 0.50, 0.65);
  leg["mc"]->SetHeader("Reco", "L");
  leg["mc"]->AddEntry(mcana_::hdphi["mc_isoR4"], "iso R4", "pl");
  leg["mc"]->AddEntry(mcana_::hdphi["mc_isoR3"], "iso R3", "pl");
  leg["gen"] = new TLegend(0.22, 0.65-0.04*3, 0.50, 0.65);
  leg["gen"]->SetHeader("Gen", "L");
  leg["gen"]->AddEntry(mcana_::hdphi["gen_isoR4"], "iso R4", "pl");
  leg["gen"]->AddEntry(mcana_::hdphi["gen_isoR3"], "iso R3", "pl");
  leg["gen_Kpi"] = new TLegend(0.22, 0.65-0.04*3, 0.50, 0.65);
  leg["gen_Kpi"]->SetHeader("Gen D^{0} #rightarrow K#pi", "L");
  leg["gen_Kpi"]->AddEntry(mcana_::hdphi["gen_Kpi_isoR4"], "iso R4", "pl");
  leg["gen_Kpi"]->AddEntry(mcana_::hdphi["gen_Kpi_isoR3"], "iso R3", "pl");
  leg["mcgen"] = new TLegend(0.22, 0.65-0.04*5, 0.50, 0.65);
  leg["mcgen"]->SetHeader("Self normalized", "L");
  leg["mcgen"]->AddEntry(mcana_::hdphi["gen_isoR4_selfnorm"], "Gen, iso R4", "pl");
  leg["mcgen"]->AddEntry(mcana_::hdphi["gen_isoR3_selfnorm"], "Gen, iso R3", "pl");
  leg["mcgen"]->AddEntry(mcana_::hdphi["mc_isoR4_selfnorm"], "Reco, iso R4", "pl");
  leg["mcgen"]->AddEntry(mcana_::hdphi["mc_isoR3_selfnorm"], "Reco, iso R3", "pl");
  for(auto& ll : leg) xjjroot::setleg(ll.second, 0.035);

  TGaxis::SetExponentOffset(-0.1, 0, "y");
  xjjroot::setgstyle(1);
  TCanvas* c;
  std::string syst = (isembed?"PYTHIA + HEDJET":"PYTHIA8");

  c = new TCanvas("c", "", 600, 600);
  mcana_::hdphi["mc_isoR4"]->Draw("pe");
  mcana_::hdphi["mc_isoR3"]->Draw("pe same");
  mcana_drawhist_::makecanvas(c, pa, leg["mc"], "chdphi_mc", syst);

  c = new TCanvas("c", "", 600, 600);
  mcana_::hdphi["gen_isoR4"]->Draw("pe");
  mcana_::hdphi["gen_isoR3"]->Draw("pe same");
  mcana_drawhist_::makecanvas(c, pa, leg["gen"], "chdphi_gen", syst);

  c = new TCanvas("c", "", 600, 600);
  mcana_::hdphi["gen_Kpi_isoR4"]->Draw("pe same");
  mcana_::hdphi["gen_Kpi_isoR3"]->Draw("pe same");
  mcana_drawhist_::makecanvas(c, pa, leg["gen_Kpi"], "chdphi_gen_Kpi", syst);

  c = new TCanvas("c", "", 600, 600);
  mcana_::hdphi["gen_isoR4_selfnorm"]->Draw("pe");
  mcana_::hdphi["gen_isoR3_selfnorm"]->Draw("pe same");
  mcana_::hdphi["mc_isoR4_selfnorm"]->Draw("pe same");
  mcana_::hdphi["mc_isoR3_selfnorm"]->Draw("pe same");
  mcana_drawhist_::makecanvas(c, pa, leg["mcgen"], "chdphi_mcgen", syst);

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc==4)
    return mcana_drawhist(argv[1], argv[2], atoi(argv[3]));
  return 1;
}

void mcana_drawhist_::seth(TH1F* h, bool forcemaxdigits)
{
  h->SetMinimum(0);
  h->SetMaximum(h->GetMaximum()*1.4);
  h->GetXaxis()->SetNdivisions(-505);
  if(forcemaxdigits) h->GetYaxis()->SetMaxDigits(1);
  xjjroot::sethempty(h, 0, 0.1);
}


void mcana_drawhist_::makecanvas(TCanvas* c, phoD::param& pa, TLegend* leg, std::string name, std::string comment)
{
  c->cd();
  pa.drawtex(0.23, 0.85, 0.035, "cent");
  leg->Draw();
  xjjroot::drawCMSleft("Simulation");
  xjjroot::drawCMSright("#sqrt{s_{NN}} = 5.02 TeV");
  xjjroot::drawtex(0.92, 0.80, comment.c_str(), 0.035, 33, 62);
  xjjroot::drawcomment(outputdir, "lb");
  std::string output = "plots/" + outputdir + "/" + name + ".pdf";
  xjjroot::mkdir(output);
  c->SaveAs(output.c_str());
  delete c;
}


