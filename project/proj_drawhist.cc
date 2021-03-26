#include <TFile.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <iostream>
#include <iomanip>

#include "param.h"

#include "xjjcuti.h"
#include "xjjrootuti.h"
#include "readcalch.h"

namespace proj_
{
  void makecanvas(TCanvas* c, phoD::param& pa, TLegend* leg, std::string name, float lumi=0, std::string comment="");
  std::string outputdir;
}

int proj_drawhist(std::string inputname_pp, std::string inputname_PbPb, std::string inputname_emix, std::string outsubdir, float mtdgain)
{
  TFile* inf_pp = TFile::Open(inputname_pp.c_str());
  if(!inf_pp) return 2;
  TFile* inf_PbPb = TFile::Open(inputname_PbPb.c_str());
  if(!inf_PbPb) return 2;
  TFile* inf_emix = TFile::Open(inputname_emix.c_str());
  if(!inf_emix) return 2;
  phoD::param pa(inf_pp);
  proj_::outputdir = outsubdir + "_" + xjjc::str_replaceall(pa.tag(), "pp_data_", "");

  phoD::readcalch r18_pp(inf_pp, "r18_pp_");
  phoD::readcalch r18_PbPb(inf_PbPb, "r18_PbPb_"), rr3_PbPb(r18_PbPb, "run3_PbPb_"), rr3mtd_PbPb(r18_PbPb, "run3mtd_PbPb_");
  phoD::readcalch r18_emix(inf_emix, "r18_emix_"), rr3_emix(r18_emix, "run3_emix_"), rr3mtd_emix(r18_emix, "run3mtd_emix_");

  float l18 = 1.7, lr3 = 7; // nb-1

  for(int i=0; i<r18_PbPb.nbin(); i++)
    {
      rr3_PbPb["hdphi_raw_effcorr_p"]->SetBinError(i+1, r18_PbPb["hdphi_raw_effcorr_p"]->GetBinError(i+1)*std::sqrt(l18/lr3));
      rr3mtd_PbPb["hdphi_raw_effcorr_p"]->SetBinError(i+1, rr3_PbPb["hdphi_raw_effcorr_p"]->GetBinError(i+1)/(1+mtdgain));
      rr3_PbPb["hdphi_bkg_effcorr_p"]->SetBinError(i+1, r18_PbPb["hdphi_bkg_effcorr_p"]->GetBinError(i+1)*std::sqrt(l18/lr3));
      rr3mtd_PbPb["hdphi_bkg_effcorr_p"]->SetBinError(i+1, rr3_PbPb["hdphi_bkg_effcorr_p"]->GetBinError(i+1)/(1+mtdgain));

      rr3_emix["hdphi_raw_effcorr_p"]->SetBinError(i+1, r18_emix["hdphi_raw_effcorr_p"]->GetBinError(i+1)*std::sqrt(l18/lr3));
      rr3mtd_emix["hdphi_raw_effcorr_p"]->SetBinError(i+1, rr3_emix["hdphi_raw_effcorr_p"]->GetBinError(i+1)/(1+mtdgain));
      rr3_emix["hdphi_bkg_effcorr_p"]->SetBinError(i+1, r18_emix["hdphi_bkg_effcorr_p"]->GetBinError(i+1)*std::sqrt(l18/lr3));
      rr3mtd_emix["hdphi_bkg_effcorr_p"]->SetBinError(i+1, rr3_emix["hdphi_bkg_effcorr_p"]->GetBinError(i+1)/(1+mtdgain));
    }
  rr3_PbPb.sub();
  rr3mtd_PbPb.sub();
  rr3_emix.sub();
  rr3mtd_emix.sub();

  phoD::subcalch s18(r18_PbPb, r18_emix, "r18_PbPb_"), sr3(rr3_PbPb, rr3_emix, "rr3_PbPb_"), sr3mtd(rr3mtd_PbPb, rr3mtd_emix, "rr3mtd_PbPb_");
  TGraphAsymmErrors* gr18 = xjjroot::shifthistcenter(s18["hdphi_sbr_effcorr"], "gr18", -0.02, "");
  TGraphAsymmErrors* grr3 = xjjroot::shifthistcenter(sr3["hdphi_sbr_effcorr"], "grr3", 0, "X0");
  TGraphAsymmErrors* grr3mtd = xjjroot::shifthistcenter(sr3mtd["hdphi_sbr_effcorr"], "grr3mtd", 0.02, "X0");

  if(s18["hdphi_sbr_effcorr"]->GetMinimum() < 0)
    r18_pp["hdphi_sbr_effcorr"]->SetMinimum(s18["hdphi_sbr_effcorr"]->GetMinimum()*2);
  if(s18["hdphi_sbr_effcorr"]->GetMaximum() > r18_pp["hdphi_sbr_effcorr"]->GetMaximum())
    r18_pp["hdphi_sbr_effcorr"]->SetMaximum(s18["hdphi_sbr_effcorr"]->GetMaximum()*1.6);

  xjjc::prt_divider();

  xjjroot::setthgrstyle(r18_pp["hdphi_sbr_effcorr"], kBlack, 20, 1, kBlack, 1, 2);
  xjjroot::setthgrstyle(s18["hdphi_sbr_effcorr"], xjjroot::mycolor_satmiddle["red"], 20, 1, xjjroot::mycolor_satmiddle["red"], 1, 2);
  xjjroot::setthgrstyle(sr3["hdphi_sbr_effcorr"], xjjroot::mycolor_satmiddle["red"], 20, 1, xjjroot::mycolor_satmiddle["red"], 1, 2);
  xjjroot::setthgrstyle(sr3mtd["hdphi_sbr_effcorr"], xjjroot::mycolor_satmiddle["red"], 20, 1, xjjroot::mycolor_satmiddle["red"], 1, 2);
  xjjroot::setthgrstyle(gr18, xjjroot::mycolor_satmiddle["red"], 20, 1, xjjroot::mycolor_satmiddle["red"], 1, 2);
  xjjroot::setthgrstyle(grr3, xjjroot::mycolor_satmiddle["blue"], 24, 1, xjjroot::mycolor_satmiddle["blue"], 1, 2);
  xjjroot::setthgrstyle(grr3mtd, xjjroot::mycolor_satmiddle["green"], 24, 1, xjjroot::mycolor_satmiddle["green"], 1, 2);

  std::map<std::string, TLegend*> leg;
  leg["18"] = new TLegend(0.22, 0.65-0.04*2, 0.50, 0.65);
  leg["18"]->AddEntry(r18_pp["hdphi_sbr_effcorr"], "pp #it{320 pb^{-1}}", "pl");
  leg["18"]->AddEntry(s18["hdphi_sbr_effcorr"], Form("PbPb #it{%s nb^{-1}}", xjjc::number_remove_zero(l18).c_str()), "pl");
  leg["r3"] = new TLegend(0.22, 0.65-0.04*2, 0.50, 0.65);
  leg["r3"]->AddEntry(r18_pp["hdphi_sbr_effcorr"], "pp #it{320 pb^{-1}}", "pl");
  leg["r3"]->AddEntry(sr3["hdphi_sbr_effcorr"], Form("PbPb #it{%s nb^{-1}}", xjjc::number_remove_zero(lr3).c_str()), "pl");
  leg["r3mtd"] = new TLegend(0.22, 0.65-0.04*2, 0.50, 0.65);
  leg["r3mtd"]->AddEntry(r18_pp["hdphi_sbr_effcorr"], "pp #it{320 pb^{-1}}", "pl");
  leg["r3mtd"]->AddEntry(sr3mtd["hdphi_sbr_effcorr"], Form("PbPb #it{%s nb^{-1}} (MTD)", xjjc::number_remove_zero(lr3).c_str()), "pl");
  leg["mtd"] = new TLegend(0.22, 0.65-0.04*4, 0.50, 0.65);
  leg["mtd"]->AddEntry(r18_pp["hdphi_sbr_effcorr"], "pp #it{320 pb^{-1}}", "pl");
  leg["mtd"]->AddEntry(gr18, Form("PbPb #it{%s nb^{-1}}", xjjc::number_remove_zero(l18).c_str()), "pl");
  leg["mtd"]->AddEntry(grr3, Form("PbPb #it{%s nb^{-1}}", xjjc::number_remove_zero(lr3).c_str()), "pl");
  leg["mtd"]->AddEntry(grr3mtd, Form("PbPb #it{%s nb^{-1}} (MTD)", xjjc::number_remove_zero(lr3).c_str()), "pl");
  for(auto& ll : leg) xjjroot::setleg(ll.second, 0.035);

  TGaxis::SetExponentOffset(-0.1, 0, "y");
  xjjroot::setgstyle(1);
  TCanvas* c;

  c = new TCanvas("c", "", 600, 600);
  r18_pp["hdphi_sbr_effcorr"]->Draw("pe");
  xjjroot::drawline(0, 0, 1, 0, kBlack, 2, 2);
  s18["hdphi_sbr_effcorr"]->Draw("pe same");
  proj_::makecanvas(c, pa, leg["18"], "chdphi_ppPbPb_18", l18, "");

  c = new TCanvas("c", "", 600, 600);
  r18_pp["hdphi_sbr_effcorr"]->Draw("pe");
  xjjroot::drawline(0, 0, 1, 0, kBlack, 2, 2);
  sr3["hdphi_sbr_effcorr"]->Draw("pe same");
  proj_::makecanvas(c, pa, leg["r3"], "chdphi_ppPbPb_r3", lr3, "");

  c = new TCanvas("c", "", 600, 600);
  r18_pp["hdphi_sbr_effcorr"]->Draw("pe");
  xjjroot::drawline(0, 0, 1, 0, kBlack, 2, 2);
  sr3mtd["hdphi_sbr_effcorr"]->Draw("pe same");
  proj_::makecanvas(c, pa, leg["r3mtd"], "chdphi_ppPbPb_r3mtd", lr3, "");

  c = new TCanvas("c", "", 600, 600);
  r18_pp["hdphi_sbr_effcorr"]->Draw("pe");
  xjjroot::drawline(0, 0, 1, 0, kBlack, 2, 2);
  gr18->Draw("pe same");
  grr3->Draw("pe same");
  grr3mtd->Draw("pe same");
  proj_::makecanvas(c, pa, leg["mtd"], "chdphi_MTD");

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc==6)
    return proj_drawhist(argv[1], argv[2], argv[3], argv[4], atof(argv[5]));
  return 1;
}

void proj_::makecanvas(TCanvas* c, phoD::param& pa, TLegend* leg, std::string name, float lumi, std::string comment)
{
  c->cd();
  pa.drawtex(0.23, 0.85, 0.035, "cent");
  leg->Draw();
  xjjroot::drawCMSleft("Projection");
  if(lumi > 0)
    // xjjroot::drawCMSright(Form("pp 320 pb^{-1}, PbPb %s nb^{-1} (5.02 TeV)", xjjc::number_remove_zero(lumi).c_str()));
    xjjroot::drawCMSright("#sqrt{s_{NN}} = 5.02 TeV");
  else
    xjjroot::drawCMSright("#sqrt{s_{NN}} = 5.02 TeV");
  xjjroot::drawtex(0.92, 0.80, comment.c_str(), 0.035, 33, 62);
  xjjroot::drawcomment(proj_::outputdir, "lb");
  std::string output = "plots/" + proj_::outputdir + "/" + name + ".pdf";
  xjjroot::mkdir(output);
  c->SaveAs(output.c_str());
  delete c;
}

