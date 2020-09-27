#ifndef __PHOD_PARAM__
#define __PHOD_PARAM__

#include <vector>
#include <string>
#include <map>
#include <iostream>

#include <TFile.h>
#include <TTree.h>

#include "xjjcuti.h"
#include "xjjrootuti.h"

namespace phoD
{
  // std::vector<float> bins_dphi = {0, 0.2, 0.4, 0.6, 0.8, 1.0};
  std::vector<float> bins_dphi = {0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0};
  int nbins_dphi = bins_dphi.size() - 1;

  // photon selection
  const float photon_pt_min_ = 40;
  const float photon_eta_abs_ = 1.442;
  const float hovere_max_ = 0.119947;
  const float see_min_ = 0;
  const float see_max_ = 0.010392;
  const float iso_max_ = 2.099277;

  class param
  {
  public:
    param(int isheavyion, float Dptmin, float Dptmax, float Dymax, float centmin, float centmax, std::string opt="");
    param(TFile* inf, std::string opt="");
    void write();
    // float v(std::string var) { return val_[var]; }
    float& operator[](std::string var) { return val_[var]; }
    bool ishi() { return (bool)ishi_; }
    std::string tag(std::string var = "") { return tag_[var]; }
    void print();
    void drawtex(float xleft, float ytop, float tsize);
  private:
    int ishi_;
    std::map<std::string, float> val_;
    std::map<std::string, std::string> tag_;
    void parsetag();
    std::string opt_;

    std::vector<std::string> list_ = {
      "Dptmin", 
      "Dptmax", 
      "Dymax", 
      "centmin", 
      "centmax", 
      "photon_pt_min", 
      "photon_eta_abs", 
      // photon_gen_iso?
    };
  };
}

phoD::param::param(int isheavyion, float Dptmin, float Dptmax, float Dymax, float centmin, float centmax, std::string opt) : opt_(opt)
{
  if(opt_!="Q") std::cout<<"\e[32;1m -- "<<__PRETTY_FUNCTION__<<std::endl;
  ishi_ = isheavyion;
  val_["Dptmin"] = Dptmin;
  val_["Dptmax"] = Dptmax;
  val_["Dymax"] = Dymax;
  val_["centmin"] = centmin;
  val_["centmax"] = centmax;

  val_["photon_pt_min"] = photon_pt_min_;
  val_["photon_eta_abs"] = photon_eta_abs_;

  parsetag();
  if(opt_!="Q") print();
}

phoD::param::param(TFile* inf, std::string opt) : opt_(opt)
{
  if(opt_!="Q") std::cout<<"\e[32;1m -- "<<__PRETTY_FUNCTION__<<std::endl;
  TTree* p = (TTree*)inf->Get("param");
  p->SetBranchAddress("ishi", &ishi_);
  for(auto pp : list_) p->SetBranchAddress(pp.c_str(), &(val_[pp]));
  p->GetEntry(0);

  parsetag();
  if(opt_!="Q") print();
}

void phoD::param::write()
{
  TTree* p = new TTree("param", "parameters");
  p->Branch("ishi", &ishi_);
  for(auto& v : val_) p->Branch(v.first.c_str(), &(v.second));
  p->Fill();
  p->Write();
}

void phoD::param::parsetag()
{
  tag_[""] = "";
  tag_[""] += (ishi_?"PbPb":"pp");
  tag_[""] += ("_pt" + xjjc::number_remove_zero(val_["Dptmin"]) + "-" + xjjc::number_remove_zero(val_["Dptmax"]));
  tag_[""] += ("_y" + xjjc::number_remove_zero(val_["Dymax"]));
  tag_[""] += ("_cent" + xjjc::number_remove_zero(val_["centmin"]) + xjjc::number_remove_zero(val_["centmax"]));

  tag_["pt"] = xjjc::number_remove_zero(val_["Dptmin"]) + " < p_{T}^{D} < " + xjjc::number_remove_zero(val_["Dptmax"]) + " GeV/c";
  tag_["y"] = "|y^{D}| < " + xjjc::number_remove_zero(val_["Dymax"]);

  tag_["cent"] = "Cent. " + xjjc::number_remove_zero(val_["centmin"]) + " - " + xjjc::number_remove_zero(val_["centmax"]) + "%";
  tag_["ishi"] = ishi_?"PbPb":"pp";

  tag_["phopt"] = "p_{T}^{#gamma} > " + xjjc::number_remove_zero(val_["photon_pt_min"]) + " GeV/c";
  tag_["phoeta"] = "|#eta^{#gamma}| < " + xjjc::number_remove_zero(val_["photon_eta_abs"]);
}

void phoD::param::print()
{
  int len = 35;
  std::cout << "\e[34m" << std::string(len, '-') << std::endl;;
  std::cout << std::left << std::setw(len) << "  "+tag_["ishi"] << "" << std::endl << std::string(len, '-') << std::endl;
  std::cout << std::left << std::setw(len) << "  "+tag_["cent"] << "" << std::endl << std::string(len, '-') << std::endl;
  std::cout << std::left << std::setw(len) << "  "+tag_["pt"] << "" << std::endl << std::string(len, '-') << std::endl;
  std::cout << std::left << std::setw(len) << "  "+tag_["y"] << "" << std::endl << std::string(len, '-') << std::endl;
  std::cout << std::left << std::setw(len) << "  "+tag_["phopt"] << "" << std::endl << std::string(len, '-') << std::endl;
  std::cout << std::left << std::setw(len) << "  "+tag_["phoeta"] << "" << std::endl << std::string(len, '-') << std::endl;
  std::cout << std::left << std::setw(len) << "  "+tag_[""] << "" << std::endl << std::string(len, '-') << std::endl;
  std::cout << "\e[0m";
}

void phoD::param::drawtex(float xleft, float ytop, float tsize)
{
  float ls = tsize*1.4;
  xjjroot::drawtex(xleft, ytop, tag_["pt"].c_str(), tsize, 13);
  xjjroot::drawtex(xleft, ytop-=ls, tag_["y"].c_str(), tsize, 13);
  xjjroot::drawtex(xleft, ytop-=ls, tag_["phopt"].c_str(), tsize, 13);
  xjjroot::drawtex(xleft, ytop-=ls, tag_["phoeta"].c_str(), tsize, 13);
}

#endif
