#include <TFile.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <iostream>
#include <iomanip>

#include "param.h"

#include "xjjcuti.h"
#include "xjjrootuti.h"
#include "xjjmypdf.h"

namespace comp_drawhist_
{
  void seth(TH1F* h, float yh=1.4, float yl=0, std::string ytitle="#frac{1}{N^{#gamma}} #frac{dN^{#gammaD}}{d#phi}");
  std::string outputdir;
  void makecanvas(TCanvas* c, phoD::param& pa, TLegend* leg, std::string name, std::string syst="PbPb", std::string comment="");
  std::map<std::string, TH1F*> hdphi;
  void geth(TFile* inf, std::string name, std::string hname, float yl=0, float yh=1.4);
}

int comp_drawhist(std::string inputname_pp, std::string inputname_PbPb, std::string inputname_emix, 
                  std::string inputname_MC_pp, std::string inputname_MC_PbPb, std::string outsubdir)
{
  TFile* inf_pp = TFile::Open(inputname_pp.c_str());
  if(!inf_pp) return 2;
  TFile* inf_PbPb = TFile::Open(inputname_PbPb.c_str());
  if(!inf_PbPb) return 2;
  TFile* inf_emix = TFile::Open(inputname_emix.c_str());
  if(!inf_emix) return 2;
  TFile* inf_MC_pp = TFile::Open(inputname_MC_pp.c_str());
  if(!inf_MC_pp) return 2;
  TFile* inf_MC_PbPb = TFile::Open(inputname_MC_PbPb.c_str());
  if(!inf_MC_PbPb) return 2;
  phoD::param pa(inf_pp);
  comp_drawhist_::outputdir = outsubdir + "_" + xjjc::str_replaceall(pa.tag(), "pp_data_", "");

  comp_drawhist_::geth(inf_pp, "sbr_effcorr_pp", "hdphi_sbr_effcorr");
  comp_drawhist_::geth(inf_PbPb, "sbr_effcorr_PbPb", "hdphi_sbr_effcorr");
  comp_drawhist_::geth(inf_emix, "sbr_effcorr_emix", "hdphi_sbr_effcorr");
  comp_drawhist_::hdphi["sbr_effcorr_PbPb_sub"] = (TH1F*)comp_drawhist_::hdphi["sbr_effcorr_PbPb"]->Clone("hdphi_sbr_effcorr_PbPb_sub");
  comp_drawhist_::hdphi["sbr_effcorr_PbPb_sub"]->Add(comp_drawhist_::hdphi["sbr_effcorr_emix"], -1);
  comp_drawhist_::seth(comp_drawhist_::hdphi["sbr_effcorr_PbPb_sub"], 1.4, 1.4);
  comp_drawhist_::geth(inf_MC_pp, "gen_isoR3_pp", "hdphi_gen_isoR3");
  TGraphErrors* gr_gen_isoR3_pp = xjjroot::shifthistcenter(comp_drawhist_::hdphi["gen_isoR3_pp"], "gr_gen_isoR3_pp", 0);
  comp_drawhist_::geth(inf_MC_pp, "gen_isoR4_pp", "hdphi_gen_isoR4");
  TGraphErrors* gr_gen_isoR4_pp = xjjroot::shifthistcenter(comp_drawhist_::hdphi["gen_isoR4_pp"], "gr_gen_isoR4_pp", 0);
  comp_drawhist_::geth(inf_MC_PbPb, "gen_isoR3_PbPb", "hdphi_gen_isoR3");
  TGraphErrors* gr_gen_isoR3_PbPb = xjjroot::shifthistcenter(comp_drawhist_::hdphi["gen_isoR3_PbPb"], "gr_gen_isoR3_PbPb", 0);
  comp_drawhist_::geth(inf_MC_PbPb, "gen_isoR4_PbPb", "hdphi_gen_isoR4");
  TGraphErrors* gr_gen_isoR4_PbPb = xjjroot::shifthistcenter(comp_drawhist_::hdphi["gen_isoR4_PbPb"], "gr_gen_isoR4_PbPb", 0);

  xjjc::prt_divider();

  std::string output = "rootfiles/" + comp_drawhist_::outputdir + "/drawhist.root";
  xjjroot::mkdir(output);
  TFile* outf = new TFile(output.c_str(), "recreate");
  for(auto& hh : comp_drawhist_::hdphi) xjjroot::writehist(hh.second);
  pa.write();
  outf->Close();

  xjjroot::setthgrstyle(comp_drawhist_::hdphi["sbr_effcorr_pp"], kBlack, 20, 1, kBlack, 1, 2);
  xjjroot::setthgrstyle(comp_drawhist_::hdphi["sbr_effcorr_PbPb"], kBlack, 20, 1, kBlack, 1, 2);
  xjjroot::setthgrstyle(comp_drawhist_::hdphi["sbr_effcorr_emix"], kBlack, 24, 1, kBlack, 1, 2);
  xjjroot::setthgrstyle(comp_drawhist_::hdphi["sbr_effcorr_PbPb_sub"], xjjroot::mycolor_middle["red"], 20, 1, xjjroot::mycolor_middle["red"], 1, 2);
  xjjroot::setthgrstyle(gr_gen_isoR3_pp, xjjroot::mycolor_middle["greenblue"], 24, 1, xjjroot::mycolor_middle["greenblue"], 7, 3);
  xjjroot::setthgrstyle(gr_gen_isoR4_pp, xjjroot::mycolor_middle["greenblue"], 20, 1, xjjroot::mycolor_middle["greenblue"], 1, 3);
  xjjroot::setthgrstyle(gr_gen_isoR3_PbPb, xjjroot::mycolor_middle["greenblue"], 24, 1, xjjroot::mycolor_middle["greenblue"], 7, 3);
  xjjroot::setthgrstyle(gr_gen_isoR4_PbPb, xjjroot::mycolor_middle["greenblue"], 20, 1, xjjroot::mycolor_middle["greenblue"], 1, 3);

  std::map<std::string, TLegend*> leg;
  leg["ppMC"] = new TLegend(0.22, 0.65-0.04*3, 0.50, 0.65);
  leg["ppMC"]->AddEntry(comp_drawhist_::hdphi["sbr_effcorr_pp"], "data", "pl");
  leg["ppMC"]->AddEntry(gr_gen_isoR3_pp, "PYTHIA8, isoR3", "l");
  leg["ppMC"]->AddEntry(gr_gen_isoR4_pp, "PYTHIA8, isoR4", "l");
  leg["PbPbMC"] = new TLegend(0.22, 0.65-0.04*3, 0.50, 0.65);
  leg["PbPbMC"]->AddEntry(comp_drawhist_::hdphi["sbr_effcorr_PbPb"], "data", "pl");
  leg["PbPbMC"]->AddEntry(gr_gen_isoR3_PbPb, "P+Hydjet, isoR3", "l");
  leg["PbPbMC"]->AddEntry(gr_gen_isoR4_PbPb, "P+Hydjet, isoR4", "l");
  leg["ppPbPb"] = new TLegend(0.22, 0.65-0.04*2, 0.50, 0.65);
  leg["ppPbPb"]->AddEntry(comp_drawhist_::hdphi["sbr_effcorr_pp"], "pp", "pl");
  leg["ppPbPb"]->AddEntry(comp_drawhist_::hdphi["sbr_effcorr_PbPb_sub"], "PbPb", "pl");
  leg["emix"] = new TLegend(0.22, 0.65-0.04*2, 0.50, 0.65);
  leg["emix"]->AddEntry(comp_drawhist_::hdphi["sbr_effcorr_PbPb"], "Trigger sample", "pl");
  leg["emix"]->AddEntry(comp_drawhist_::hdphi["sbr_effcorr_emix"], "Mixed events", "pl");
  leg["emix_sub"] = new TLegend(0.22, 0.65-0.04, 0.50, 0.65);
  leg["emix_sub"]->AddEntry(comp_drawhist_::hdphi["sbr_effcorr_PbPb_sub"], "After sub.", "pl");
  for(auto& ll : leg) xjjroot::setleg(ll.second, 0.035);

  TGaxis::SetExponentOffset(-0.1, 0, "y");
  xjjroot::setgstyle(1);
  std::string output = "plots/" + outputdir + "/.pdf";
  TCanvas* c;

  c = new TCanvas("c", "", 600, 600);
  comp_drawhist_::hdphi["sbr_effcorr_pp"]->Draw("pe");
  gr_gen_isoR3_pp->Draw("lX0 same");
  gr_gen_isoR4_pp->Draw("lX0 same");
  comp_drawhist_::makecanvas(c, pa, leg["ppMC"], "chdphi_ppMC", "pp", "");

  c = new TCanvas("c", "", 600, 600);
  comp_drawhist_::hdphi["sbr_effcorr_PbPb"]->Draw("pe");
  gr_gen_isoR3_PbPb->Draw("lX0 same");
  gr_gen_isoR4_PbPb->Draw("lX0 same");
  comp_drawhist_::makecanvas(c, pa, leg["PbPbMC"], "chdphi_PbPbMC", "PbPb", "");

  c = new TCanvas("c", "", 600, 600);
  comp_drawhist_::hdphi["sbr_effcorr_PbPb"]->Draw("pe");
  comp_drawhist_::hdphi["sbr_effcorr_emix"]->Draw("pe same");
  comp_drawhist_::makecanvas(c, pa, leg["emix"], "chdphi_emix", "PbPb", "");

  c = new TCanvas("c", "", 600, 600);
  comp_drawhist_::hdphi["sbr_effcorr_PbPb_sub"]->Draw("pe");
  comp_drawhist_::hdphi["sbr_effcorr_pp"]->Draw("pe same");
  comp_drawhist_::makecanvas(c, pa, leg["ppPbPb"], "chdphi_ppPbPb", "", "");

  c = new TCanvas("c", "", 600, 600);
  comp_drawhist_::hdphi["sbr_effcorr_PbPb_sub"]->Draw("pe");
  comp_drawhist_::makecanvas(c, pa, leg["emix_sub"], "chdphi_PbPb_sub", "", "");

  return 0;
}

void comp_drawhist_::seth(TH1F* h, float yl, float yh, std::string ytitle)
{
  xjjroot::sethempty(h, 0, 0.1);
  h->SetMinimum(h->GetMinimum()*yl);
  h->SetMaximum(h->GetMaximum()*yh);
  h->GetXaxis()->SetNdivisions(-505);
  // if(forcemaxdigits) h->GetYaxis()->SetMaxDigits(1);
  h->GetYaxis()->SetTitle(ytitle.c_str());
}

int main(int argc, char* argv[])
{
  if(argc==7)
    return comp_drawhist(argv[1], argv[2], argv[3], argv[4], argv[5], argv[6]);
  return 1;
}

void comp_drawhist_::makecanvas(TCanvas* c, phoD::param& pa, TLegend* leg, std::string name, std::string syst, std::string comment)
{
  c->cd();
  pa.drawtex(0.23, 0.85, 0.035, "cent");
  leg->Draw();
  xjjroot::drawCMSleft();
  xjjroot::drawCMSright(Form("%s #sqrt{s_{NN}} = 5.02 TeV", syst.c_str()));
  xjjroot::drawtex(0.92, 0.80, comment.c_str(), 0.035, 33, 62);
  xjjroot::drawcomment(comp_drawhist_::outputdir, "lb");
  std::string output = "plots/" + comp_drawhist_::outputdir + "/" + name + ".pdf";
  xjjroot::mkdir(output);
  c->SaveAs(output.c_str());
  delete c;
}

void comp_drawhist_::geth(TFile* inf, std::string name, std::string hname, float yl, float yh)
{
  std::cout<<"\e[0m"<<__FUNCTION__<<" --> "<<name<<"\e[0m"<<std::endl;
  hdphi[name] = (TH1F*)inf->Get(hname.c_str());
  hdphi[name]->SetName(Form("hdphi_%s", name.c_str()));  
  seth(hdphi[name], yl, yh);
}
