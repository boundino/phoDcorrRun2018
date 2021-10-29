#ifndef __PHOD_PARAM__
#define __PHOD_PARAM__

#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <iomanip>

#include <TFile.h>
#include <TTree.h>

#include "xjjcuti.h"
#include "xjjrootuti.h"

namespace Djet
{
  std::vector<std::string> var = {"dphi", "dr"};
  std::map<std::string, std::string> vartex = {
    {"dphi", "#Delta#phi^{jD} / #pi"},
    {"dr", "#Deltar^{jD}"}
  };
  std::map<std::string, std::string> varytex = {
    {"dphi", "#frac{dN^{jD}}{d#Delta#phi}"},
    {"dr", "#frac{dN^{jD}}{d#Deltar}"}
  };
  std::map<std::string, std::vector<float>> bins_pp = {
    {"dphi", {0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0}},
    {"dr", {0, 0.05, 0.1, 0.15, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 1.0}},
  };
  std::map<std::string, std::vector<float>> bins_aa = {
    // {"dphi", {0, 0.2, 0.4, 0.6, 0.8, 1.0}},
    // {"dr", {0, 0.1, 0.2, 0.4, 0.6, 0.8, 1.0}},
    {"dphi", {0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0}},
    {"dr", {0, 0.05, 0.1, 0.15, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 1.0}},
  };
}

namespace Djet
{
  // jet selection
  // const float iso_max_aa_gen_ = 5;

  class param
  {
  public:
    param() { ; }
    param(int isheavyion, float Dptmin, float Dptmax, float Dymax, float centmin, float centmax, float jtptmin, float jtetamax, int ismc, std::string opt="");
    param(TFile* inf, std::string opt="");
    void write();
    // float v(std::string var) { return val_[var]; }
    float& operator[](std::string var) { return val_[var]; }
    bool ishi() { return (bool)ishi_; }
    bool ismc() { return (bool)ismc_; }
    void setishi(int ishi__) { ishi_ = ishi__; parsetag(); }
    void setismc(int ismc__) { ismc_ = ismc__; parsetag(); }
    std::string tag(std::string var = "") { return tag_[var]; }
    void print();
    void drawtex(float xleft, float ytop, float tsize, std::string exclude="", float xoff2=0.5, float yoff2=0);
  private:
    int ishi_, ismc_;
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
      "jtptmin", 
      "jtetamax", 
    };
  };
}

Djet::param::param(int isheavyion, float Dptmin, float Dptmax, float Dymax, 
                   float centmin, float centmax, 
                   float jtptmin, float jtetamax, int ismc,
                   std::string opt) : opt_(opt)
{
  if(opt_!="Q") std::cout<<"\e[32;1m -- "<<__PRETTY_FUNCTION__<<"\e[0m"<<std::endl;
  ishi_ = isheavyion;
  ismc_ = ismc;
  val_["Dptmin"] = Dptmin;
  val_["Dptmax"] = Dptmax;
  val_["Dymax"] = Dymax;
  val_["centmin"] = centmin;
  val_["centmax"] = centmax;

  val_["jtptmin"] = jtptmin;
  val_["jtetamax"] = jtetamax;

  parsetag();
  if(opt_!="Q") print();
}

Djet::param::param(TFile* inf, std::string opt) : opt_(opt)
{
  if(opt_!="Q") std::cout<<"\e[32;1m -- "<<__PRETTY_FUNCTION__<<"\e[0m"<<std::endl;
  TTree* p = (TTree*)inf->Get("param");
  p->SetBranchAddress("ishi", &ishi_);
  p->SetBranchAddress("ismc", &ismc_);
  for(auto pp : list_) p->SetBranchAddress(pp.c_str(), &(val_[pp]));
  p->GetEntry(0);

  parsetag();
  if(opt_ != "Q") print();
}

void Djet::param::write()
{
  TTree* p = new TTree("param", "parameters");
  p->Branch("ishi", &ishi_);
  p->Branch("ismc", &ismc_);
  for(auto& v : val_) p->Branch(v.first.c_str(), &(v.second));
  p->Fill();
  p->Write();
}

void Djet::param::parsetag()
{
  tag_[""] = "";
  std::string tishi = (ishi_==0?"pp":(ishi_==1?"PbPb":""));
  tag_[""] += tishi;
  std::string tismc = (ismc_==0?"data":(ismc_==1?"MC":""));
  tag_[""] += (((tag_[""]=="" || tismc=="")?"":"_")+tismc);
  tag_[""] += ((tag_[""]==""?"":"_") + ("jtpt" + xjjc::number_remove_zero(val_["jtptmin"])));
  tag_[""] += ("_jteta" + xjjc::number_remove_zero(val_["jtetamax"]));
  tag_[""] += ("_pt" + xjjc::number_remove_zero(val_["Dptmin"]) + "-" + xjjc::number_remove_zero(val_["Dptmax"]));
  tag_[""] += ("_y" + xjjc::number_remove_zero(val_["Dymax"]));
  if(ishi_==1) 
    tag_[""] += ("_cent" + xjjc::number_remove_zero(val_["centmin"]) + xjjc::number_remove_zero(val_["centmax"]));

  tag_["pt"] = xjjc::number_remove_zero(val_["Dptmin"]) + " < p_{T}^{D} < " + xjjc::number_remove_zero(val_["Dptmax"]) + " GeV/c";
  tag_["y"] = "|y^{D}| < " + xjjc::number_remove_zero(val_["Dymax"]);

  if(ishi_) tag_["cent"] = "Cent. " + xjjc::number_remove_zero(val_["centmin"]) + " - " + xjjc::number_remove_zero(val_["centmax"]) + "%";
  else tag_["cent"] = "";
  tag_["ishi"] = tishi;
  tag_["ismc"] = tismc;

  tag_["jtpt"] = "p_{T}^{jet} > " + xjjc::number_remove_zero(val_["jtptmin"]) + " GeV/c";
  tag_["jteta"] = "|#eta^{jet}| < " + xjjc::number_remove_zero(val_["jtetamax"]);
}

void Djet::param::print()
{
  xjjc::prt_divider("\e[34m");
  for(auto& t : tag_)
    {
      std::cout << "\e[34m  " << t.second << "\e[0m" << std::endl;
      xjjc::prt_divider("\e[34m");
    }
}

void Djet::param::drawtex(float xleft, float ytop, float tsize, std::string exclude, float xoff2, float yoff2)
{
  std::vector<std::string> ex = xjjc::str_divide(exclude, ",");
  float ls = tsize*1.4;

  float ytop_ = ytop + ls;
  std::vector<std::string> info_left = {"pt", "y", "jtpt", "jteta"};
  for(auto& i : info_left)
    if(std::find(ex.begin(), ex.end(), i) == ex.end())
      { xjjroot::drawtex(xleft, ytop_-=ls, tag_[i].c_str(), tsize, 13); }

  std::vector<std::string> info_right = {"cent"};
  ytop_ = ytop + yoff2 + ls;
  for(auto& i : info_right)
    if(std::find(ex.begin(), ex.end(), i) == ex.end())
      { xjjroot::drawtex(xleft+xoff2, ytop_-=ls, tag_[i].c_str(), tsize, 13); }
}

namespace phoD
{
  std::vector<std::string> var = {"dphi"};
  std::map<std::string, std::string> vartex = {
    {"dphi", "#Delta#phi^{jD} / #pi"},
  };
  std::map<std::string, std::string> varytex = {
    {"dphi", "#frac{dN^{jD}}{d#Delta#phi}"},
  };
  std::map<std::string, std::vector<float>> bins_pp = {
    {"dphi", {0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0}},
  };
  std::map<std::string, std::vector<float>> bins_aa = {
    {"dphi", {0, 0.3, 0.5, 0.7, 0.9, 1.0}},
  };

  // photon selection
  const float hovere_max_pp_ = 0.009732;
  const float hovere_max_aa_ = 0.119947;
  const float see_min_pp_raw_ = 0;
  const float see_max_pp_raw_ = 0.009905;
  const float see_min_pp_bkg_ = 0.012;
  const float see_max_pp_bkg_ = 0.02;
  const float see_min_aa_raw_ = 0;
  const float see_max_aa_raw_ = 0.010392;
  const float see_min_aa_bkg_ = 0.012;
  // const float see_min_aa_bkg_ = 0.01;
  const float see_max_aa_bkg_ = 0.02;
  const float iso_max_pp_ = -0.014755;
  const float iso_max_aa_ = 2.099277;
  const float iso_max_pp_gen_ = 5;
  const float iso_max_aa_gen_ = 5;

  // photon purity
  const float purity_pp_ = 0.844;
  const float purity_aa_ = 0.767;
}

namespace phoD
{
  class param
  {
  public:
    param() { ; }
    param(int isheavyion, float Dptmin, float Dptmax, float Dymax, float centmin, float centmax, float phoptmin, float phoetamax, int ismc, std::string opt="");
    param(TFile* inf, std::string opt="");
    void write();
    // float v(std::string var) { return val_[var]; }
    float& operator[](std::string var) { return val_[var]; }
    bool ishi() { return (bool)ishi_; }
    std::string tag(std::string var = "") { return tag_[var]; }
    void print();
    void drawtex(float xleft, float ytop, float tsize, std::string exclude="");
  private:
    int ishi_, ismc_;
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
      "phoptmin", 
      "phoetamax", 
    };
  };
}

phoD::param::param(int isheavyion, float Dptmin, float Dptmax, float Dymax, 
                   float centmin, float centmax, 
                   float phoptmin, float phoetamax, int ismc,
                   std::string opt) : opt_(opt)
{
  if(opt_!="Q") std::cout<<"\e[32;1m -- "<<__PRETTY_FUNCTION__<<"\e[0m"<<std::endl;
  ishi_ = isheavyion;
  ismc_ = ismc;
  val_["Dptmin"] = Dptmin;
  val_["Dptmax"] = Dptmax;
  val_["Dymax"] = Dymax;
  val_["centmin"] = centmin;
  val_["centmax"] = centmax;

  val_["phoptmin"] = phoptmin;
  val_["phoetamax"] = phoetamax;

  parsetag();
  if(opt_!="Q") print();
}

phoD::param::param(TFile* inf, std::string opt) : opt_(opt)
{
  if(opt_!="Q") std::cout<<"\e[32;1m -- "<<__PRETTY_FUNCTION__<<"\e[0m"<<std::endl;
  TTree* p = (TTree*)inf->Get("param");
  p->SetBranchAddress("ishi", &ishi_);
  p->SetBranchAddress("ismc", &ismc_);
  for(auto pp : list_) p->SetBranchAddress(pp.c_str(), &(val_[pp]));
  p->GetEntry(0);

  parsetag();
  if(opt_ != "Q") print();
}

void phoD::param::write()
{
  TTree* p = new TTree("param", "parameters");
  p->Branch("ishi", &ishi_);
  p->Branch("ismc", &ismc_);
  for(auto& v : val_) p->Branch(v.first.c_str(), &(v.second));
  p->Fill();
  p->Write();
}

void phoD::param::parsetag()
{
  tag_[""] = "";
  tag_[""] += (ishi_?"PbPb":"pp");
  tag_[""] += (ismc_?"_MC":"_data");
  tag_[""] += ("_phopt" + xjjc::number_remove_zero(val_["phoptmin"]));
  tag_[""] += ("_phoeta" + xjjc::number_remove_zero(val_["phoetamax"]));
  tag_[""] += ("_pt" + xjjc::number_remove_zero(val_["Dptmin"]) + "-" + xjjc::number_remove_zero(val_["Dptmax"]));
  tag_[""] += ("_y" + xjjc::number_remove_zero(val_["Dymax"]));
  if(ishi_) tag_[""] += ("_cent" + xjjc::number_remove_zero(val_["centmin"]) + xjjc::number_remove_zero(val_["centmax"]));

  tag_["pt"] = xjjc::number_remove_zero(val_["Dptmin"]) + " < p_{T}^{D} < " + xjjc::number_remove_zero(val_["Dptmax"]) + " GeV/c";
  tag_["y"] = "|y^{D}| < " + xjjc::number_remove_zero(val_["Dymax"]);

  if(ishi_) tag_["cent"] = "Cent. " + xjjc::number_remove_zero(val_["centmin"]) + " - " + xjjc::number_remove_zero(val_["centmax"]) + "%";
  else tag_["cent"] = "";
  tag_["ishi"] = ishi_?"PbPb":"pp";
  tag_["ismc"] = ismc_?"MC":"data";

  tag_["phopt"] = "p_{T}^{#gamma} > " + xjjc::number_remove_zero(val_["phoptmin"]) + " GeV/c";
  tag_["phoeta"] = "|#eta^{#gamma}| < " + xjjc::number_remove_zero(val_["phoetamax"]);
}

void phoD::param::print()
{
  xjjc::prt_divider("\e[34m");
  for(auto& t : tag_)
    {
      std::cout << "\e[34m  " << t.second << "\e[0m" << std::endl;
      xjjc::prt_divider("\e[34m");
    }
}

void phoD::param::drawtex(float xleft, float ytop, float tsize, std::string exclude)
{
  std::vector<std::string> ex = xjjc::str_divide(exclude, ",");
  float ls = tsize*1.4;

  float ytop_ = ytop + ls;
  std::vector<std::string> info_left = {"pt", "y", "phopt", "phoeta"};
  for(auto& i : info_left)
    if(std::find(ex.begin(), ex.end(), i) == ex.end())
      { xjjroot::drawtex(xleft, ytop_-=ls, tag_[i].c_str(), tsize, 13); }

  std::vector<std::string> info_right = {"cent"};
  ytop_ = ytop + ls;
  for(auto& i : info_right)
    if(std::find(ex.begin(), ex.end(), i) == ex.end())
      { xjjroot::drawtex(xleft+0.5, ytop_-=ls, tag_[i].c_str(), tsize, 13); }
}

#endif
