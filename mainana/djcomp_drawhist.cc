#include <TFile.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <iostream>
#include <iomanip>

#include "para.h"

#include "xjjcuti.h"
#include "xjjrootuti.h"
#include "xjjanauti.h"
#include "xjjmypdf.h"
#include "bins.h"

namespace comp_
{
  void seth(TH1F* h, float yh=1.4, float yl=0);
  void makecanvas(xjjroot::mypdf* f, Djet::param& pa, TLegend* leg, std::string syst="PbPb", std::string comment="");
  float setyminmax_linear(std::vector<TH1F*> h);
  float setyminmax_log(std::vector<TH1F*> h);
}

int comp_drawhist(std::string outsubdir, std::string inputname_pp="", std::string inputname_PbPb="", std::string inputname_emix="", 
                  std::string inputname_MC_pp="", std::string inputname_MC_PbPb="")
{
  std::map<std::string, std::string> inputname = 
    {
      {"pp", inputname_pp},
      {"PbPb", inputname_PbPb},
      {"emix", inputname_emix},
      {"MC_pp", inputname_MC_pp},
      {"MC_PbPb", inputname_MC_PbPb},
    };
  std::map<std::string, bool> status;
  std::map<std::string, TFile*> inf;
  for(auto& i : inputname)
    {
      inf[i.first] = TFile::Open(i.second.c_str()); 
      status[i.first] = (inf[i.first] != 0);
      std::cout<<" "<<(status[i.first]?"\e[36m-->":"\e[31m(x)")<<" "<<i.second<<"\e[0m"<<std::endl;
    }

  if(!status["PbPb"] && !status["pp"]) return 3;
  Djet::param pa;
  if(status["PbPb"]) pa = Djet::param(inf["PbPb"]);
  else pa = Djet::param(inf["pp"]);
  pa.setismc(2);
  std::string outputdir = outsubdir + "_" + pa.tag();

  std::map<std::string, std::map<std::string, TH1F*>> hd;
  std::map<std::string, std::map<std::string, TGraphErrors*>> gr;
  for(auto& v : Djet::var)
    {
      for(auto& s : status)
        {
          if(!s.second) continue;
          if(xjjc::str_contains(s.first, "MC"))
            hd[v][s.first] = xjjroot::gethist<TH1F>(inf[s.first], "h"+v+"_gen");
          else
            hd[v][s.first] = xjjroot::gethist<TH1F>(inf[s.first], "h"+v+"_sbr_effcorr");
          hd[v][s.first]->SetName(Form("h%s_%s", v.c_str(), s.first.c_str()));
        }
      if(status["emix"])
        {
          hd[v]["emix_avg"] = (TH1F*)hd[v]["emix"]->Clone(Form("h%s_emix_avg", v.c_str()));
          // xjjana::bins<double> vbx(xjjana::gethXaxis(hd[v]["emix"]));
          // float bincontent = 0, normbin = 0;
          // for(int i=0; i<xjjana::gethXn(hd[v]["emix"]); i++)
          //   {
          //     bincontent += (hd[v]["emix"]->GetBinContent(i+1)*(v=="dr"?vbx.area(i):vbx.width(i)));
          //     normbin += (v=="dr"?vbx.area(i):vbx.width(i));
          //   }
          double bincontent = 1.e+10;
          for(int i=0; i<xjjana::gethXn(hd[v]["emix"]); i++)
            bincontent = std::min(bincontent, hd[v]["emix"]->GetBinContent(i+1));
          for(int i=0; i<xjjana::gethXn(hd[v]["emix"]); i++)
            {
              hd[v]["emix_avg"]->SetBinContent(i+1, bincontent);
              if(hd[v]["emix"]->GetBinContent(i+1) > 0)
                hd[v]["emix_avg"]->SetBinError(i+1, hd[v]["emix"]->GetBinError(i+1) / hd[v]["emix"]->GetBinContent(i+1) * bincontent);
              else
                hd[v]["emix_avg"]->SetBinError(i+1, 0);
            }
        }
      if(status["PbPb"] && status["emix"])
        {
          hd[v]["sub"] = (TH1F*)hd[v]["PbPb"]->Clone(Form("h%s_sub", v.c_str()));
          hd[v]["sub_avg"] = (TH1F*)hd[v]["PbPb"]->Clone(Form("h%s_sub_avg", v.c_str()));
          for(int i=0; i<xjjana::gethXn(hd[v]["sub"]); i++)
            {
              hd[v]["sub"]->SetBinContent(i+1, hd[v]["PbPb"]->GetBinContent(i+1) - hd[v]["emix"]->GetBinContent(i+1));
              hd[v]["sub"]->SetBinError(i+1, hd[v]["PbPb"]->GetBinError(i+1));
              hd[v]["sub_avg"]->SetBinContent(i+1, hd[v]["PbPb"]->GetBinContent(i+1) - hd[v]["emix_avg"]->GetBinContent(i+1));
              hd[v]["sub_avg"]->SetBinError(i+1, hd[v]["PbPb"]->GetBinError(i+1));
            }
        }
    }
  status["emix_avg"] = status["emix"];
  status["sub"] = status["PbPb"] && status["emix"];
  status["sub_avg"] = status["PbPb"] && status["emix_avg"];
  // Self-normalized
  for(auto& t : {"pp", "sub", "MC_pp", "sub_avg"})
    {
      if(status[t])
        {
          hd["dphi"][Form("%s_selfnorm", t)] = (TH1F*)hd["dphi"][t]->Clone(Form("%s_selfnorm", hd["dphi"][t]->GetName()));
          hd["dphi"][Form("%s_selfnorm", t)]->Scale(1./hd["dphi"][t]->Integral("width"));
          hd["dphi"][Form("%s_selfnorm", t)]->GetYaxis()->SetTitle(xjjc::str_replaceall(hd["dphi"][t]->GetYaxis()->GetTitle(), "jet", "jD").c_str());
          hd["dr"][Form("%s_selfnorm", t)] = (TH1F*)hd["dr"][t]->Clone(Form("%s_selfnorm", hd["dr"][t]->GetName()));
          xjjana::bins<double> vbx(xjjana::gethXaxis(hd["dr"][t]));
          // xjjana::bins<double> vbx(std::vector<double>(hd["dr"][t]->GetXaxis()->GetXbins()->GetArray(), hd["dr"][t]->GetXaxis()->GetXbins()->GetArray() + hd["dr"][t]->GetXaxis()->GetNbins()+1));
          float drnorm = 0;
          for(int i=0; i<hd["dr"][t]->GetXaxis()->FindBin(0.399); i++)
            drnorm += (hd["dr"][t]->GetBinContent(i+1)*vbx.area(i)/(0.4*0.4));
          hd["dr"][Form("%s_selfnorm", t)]->Scale(1./drnorm);
          hd["dr"][Form("%s_selfnorm", t)]->GetYaxis()->SetTitle(xjjc::str_replaceall(hd["dr"][t]->GetYaxis()->GetTitle(), "jet", "jD").c_str());
          status[Form("%s_selfnorm", t)] = true;
        }
      else
        status[Form("%s_selfnorm", t)] = false;
    }
  for(auto& v : Djet::var)
    for(auto& s : status)
      {
        if(!s.second) continue;
        gr[v][s.first] = xjjana::shifthistcenter(hd[v][s.first], Form("gr%s", hd[v][s.first]->GetName()), 0);
        comp_::seth(hd[v][s.first], 1.4, 1.4);
      }

  xjjc::prt_divider();

  std::string output = "rootfiles/" + outputdir + "/drawhist.root";
  xjjroot::mkdir(output);
  TFile* outf = new TFile(output.c_str(), "recreate");
  for(auto& v : Djet::var)
    for(auto& hh : hd[v])
      xjjroot::writehist(hh.second);
  pa.write();
  outf->Close();

  xjjc::prt_divider();

  for(auto& v : Djet::var)
    {
      if(status["pp"])
        xjjroot::setthgrstyle(gr[v]["pp"], kBlack, 20, 1, kBlack, 1, 2);
      if(status["pp_selfnorm"])
        xjjroot::setthgrstyle(gr[v]["pp_selfnorm"], kBlack, 20, 1, kBlack, 1, 2);
      if(status["PbPb"])
        xjjroot::setthgrstyle(gr[v]["PbPb"], kBlack, 21, 1, kBlack, 1, 2);
      if(status["emix"])
        xjjroot::setthgrstyle(gr[v]["emix"], kBlack, 25, 1, kBlack, 1, 2);
      if(status["emix_avg"])
        xjjroot::setthgrstyle(gr[v]["emix_avg"], kBlack, 25, 1, kBlack, 1, 2);
      if(status["sub"])
        xjjroot::setthgrstyle(gr[v]["sub"], xjjroot::mycolor_satmiddle["red"], 21, 1, xjjroot::mycolor_satmiddle["red"], 1, 2);
      if(status["sub_selfnorm"])
        xjjroot::setthgrstyle(gr[v]["sub_selfnorm"], xjjroot::mycolor_satmiddle["red"], 21, 1, xjjroot::mycolor_satmiddle["red"], 1, 2);
      if(status["sub_avg"])
        xjjroot::setthgrstyle(gr[v]["sub_avg"], xjjroot::mycolor_satmiddle["red"], 21, 1, xjjroot::mycolor_satmiddle["red"], 1, 2);
      if(status["sub_avg_selfnorm"])
        xjjroot::setthgrstyle(gr[v]["sub_avg_selfnorm"], xjjroot::mycolor_satmiddle["red"], 21, 1, xjjroot::mycolor_satmiddle["red"], 1, 2);
      if(status["MC_pp"])
        xjjroot::setthgrstyle(gr[v]["MC_pp"], xjjroot::mycolor_satmiddle["azure"], 20, 1, xjjroot::mycolor_satmiddle["azure"], 1, 3);
      if(status["MC_pp_selfnorm"])
        xjjroot::setthgrstyle(gr[v]["MC_pp_selfnorm"], xjjroot::mycolor_satmiddle["azure"], 20, 1, xjjroot::mycolor_satmiddle["azure"], 1, 3);
      if(status["MC_PbPb"])
        xjjroot::setthgrstyle(gr[v]["MC_PbPb"], xjjroot::mycolor_satmiddle["azure"], 20, 1, xjjroot::mycolor_satmiddle["azure"], 1, 3);
    }

  std::map<std::string, TLegend*> leg;
  if(status["pp"] && status["MC_pp"])
    {
      leg["ppMC"] = new TLegend(0.25, 0.85-0.04*2, 0.54, 0.85);
      leg["ppMC"]->AddEntry(gr["dphi"]["pp"], "data", "pl");
      leg["ppMC"]->AddEntry(gr["dphi"]["MC_pp"], "PYTHIA8", "l");
    }
  if(status["PbPb"] && status["MC_PbPb"])
    {
      leg["PbPbMC"] = new TLegend(0.25, 0.85-0.04*2, 0.54, 0.85);
      leg["PbPbMC"]->AddEntry(gr["dphi"]["PbPb"], "data", "pl");
      leg["PbPbMC"]->AddEntry(gr["dphi"]["MC_PbPb"], "P+Hydjet", "l");
    }
  if(status["pp"] && status["sub"])
    {
      leg["ppPbPb"] = new TLegend(0.25, 0.85-0.04*2, 0.54, 0.85);
      leg["ppPbPb"]->AddEntry(gr["dphi"]["pp"], "pp", "pl");
      leg["ppPbPb"]->AddEntry(gr["dphi"]["sub"], "PbPb", "pl");
    }
  if(status["pp"] && status["MC_pp"] && status["sub"])
    {
      leg["ppMCPbPb"] = new TLegend(0.25, 0.85-0.04*3, 0.54, 0.85);
      leg["ppMCPbPb"]->AddEntry(gr["dphi"]["pp"], "pp", "pl");
      leg["ppMCPbPb"]->AddEntry(gr["dphi"]["MC_pp"], "PYTHIA8", "l");
      leg["ppMCPbPb"]->AddEntry(gr["dphi"]["sub"], "PbPb (bkg sub)", "pl");
    }
  if(status["pp"] && status["MC_pp"] && status["sub_avg"])
    {
      leg["ppMCPbPb_avg"] = new TLegend(0.25, 0.85-0.04*3, 0.54, 0.85);
      leg["ppMCPbPb_avg"]->AddEntry(gr["dphi"]["pp"], "pp", "pl");
      leg["ppMCPbPb_avg"]->AddEntry(gr["dphi"]["MC_pp"], "PYTHIA8", "l");
      leg["ppMCPbPb_avg"]->AddEntry(gr["dphi"]["sub_avg"], "PbPb (bkg sub)", "pl");
    }
  if(status["PbPb"] && status["emix"])
    {
      leg["emix"] = new TLegend(0.25, 0.85-0.04*2, 0.54, 0.85);
      leg["emix"]->AddEntry(gr["dphi"]["PbPb"], "Trigger sample", "pl");
      leg["emix"]->AddEntry(gr["dphi"]["emix"], "Mixed events", "pl");
    }
  if(status["PbPb"] && status["emix_avg"])
    {
      leg["emix_avg"] = new TLegend(0.25, 0.85-0.04*2, 0.54, 0.85);
      leg["emix_avg"]->AddEntry(gr["dphi"]["PbPb"], "Trigger sample", "pl");
      leg["emix_avg"]->AddEntry(gr["dphi"]["emix_avg"], "Mixed events", "pl");
    }
  for(auto& ll : leg) xjjroot::setleg(ll.second, 0.035);

  TGaxis::SetExponentOffset(-0.1, 0, "y");
  xjjroot::setgstyle(1);

  for(auto& v : Djet::var)
    {
      xjjroot::mypdf* fpdf = new xjjroot::mypdf("plots/" + outputdir + "/ccomp_" + v + ".pdf", "c", 600, 600);
      if(leg.find("ppMC") != leg.end())
        {
          comp_::setyminmax_linear({hd[v]["MC_pp"], hd[v]["pp"]});
          fpdf->prepare();
          fpdf->getc()->SetLogy(0);
          hd[v]["pp"]->Draw("AXIS");
          gr[v]["MC_pp"]->Draw("lX0 same");
          gr[v]["pp"]->Draw("pe same");
          comp_::makecanvas(fpdf, pa, leg["ppMC"], "pp");

          comp_::setyminmax_log({hd[v]["MC_pp"], hd[v]["pp"]});
          fpdf->prepare();
          fpdf->getc()->SetLogy();
          hd[v]["pp"]->Draw("AXIS");
          gr[v]["MC_pp"]->Draw("lX0 same");
          gr[v]["pp"]->Draw("pe same");
          comp_::makecanvas(fpdf, pa, leg["ppMC"], "pp");

        }
      if(leg.find("PbPbMC") != leg.end())
        {
          comp_::setyminmax_linear({hd[v]["MC_PbPb"], hd[v]["PbPb"]});
          fpdf->prepare();
          fpdf->getc()->SetLogy(0);
          hd[v]["PbPb"]->Draw("AXIS");
          gr[v]["MC_PbPb"]->Draw("lX0 same");
          gr[v]["PbPb"]->Draw("pe same");
          comp_::makecanvas(fpdf, pa, leg["PbPbMC"], "PbPb");

          comp_::setyminmax_log({hd[v]["MC_PbPb"], hd[v]["PbPb"]});
          fpdf->prepare();
          fpdf->getc()->SetLogy();
          hd[v]["PbPb"]->Draw("AXIS");
          gr[v]["MC_PbPb"]->Draw("lX0 same");
          gr[v]["PbPb"]->Draw("pe same");
          comp_::makecanvas(fpdf, pa, leg["PbPbMC"], "PbPb");
        }
      if(leg.find("emix") != leg.end())
        {
          comp_::setyminmax_linear({hd[v]["PbPb"], hd[v]["emix"]});
          fpdf->prepare();
          fpdf->getc()->SetLogy(0);
          hd[v]["PbPb"]->Draw("AXIS");
          gr[v]["PbPb"]->Draw("pe same");
          gr[v]["emix"]->Draw("pe same");
          comp_::makecanvas(fpdf, pa, leg["emix"], "PbPb");

          comp_::setyminmax_log({hd[v]["PbPb"], hd[v]["emix"]});
          fpdf->prepare();
          fpdf->getc()->SetLogy();
          hd[v]["PbPb"]->Draw("AXIS");
          gr[v]["PbPb"]->Draw("pe same");
          gr[v]["emix"]->Draw("pe same");
          comp_::makecanvas(fpdf, pa, leg["emix"], "PbPb");
        }
      if(leg.find("emix_avg") != leg.end())
        {
          comp_::setyminmax_linear({hd[v]["PbPb"], hd[v]["emix_avg"]});
          fpdf->prepare();
          fpdf->getc()->SetLogy(0);
          hd[v]["PbPb"]->Draw("AXIS");
          gr[v]["PbPb"]->Draw("pe same");
          gr[v]["emix_avg"]->Draw("pe same");
          comp_::makecanvas(fpdf, pa, leg["emix_avg"], "PbPb");

          comp_::setyminmax_log({hd[v]["PbPb"], hd[v]["emix_avg"]});
          fpdf->prepare();
          fpdf->getc()->SetLogy();
          hd[v]["PbPb"]->Draw("AXIS");
          gr[v]["PbPb"]->Draw("pe same");
          gr[v]["emix_avg"]->Draw("pe same");
          comp_::makecanvas(fpdf, pa, leg["emix_avg"], "PbPb");
        }
      if(leg.find("ppPbPb") != leg.end())
        {
          float ymin = comp_::setyminmax_linear({hd[v]["pp"], hd[v]["sub"]});
          fpdf->prepare();
          fpdf->getc()->SetLogy(0);
          hd[v]["pp"]->Draw("AXIS");
          if(ymin < 0) 
            xjjroot::drawline(hd[v]["pp"]->GetXaxis()->GetXmin(), 0, hd[v]["pp"]->GetXaxis()->GetXmax(), 0, kBlack, 2, 2);
          gr[v]["sub"]->Draw("pe same");
          gr[v]["pp"]->Draw("pe same");
          comp_::makecanvas(fpdf, pa, leg["ppPbPb"], "");

          comp_::setyminmax_log({hd[v]["pp"], hd[v]["sub"]});
          fpdf->prepare();
          fpdf->getc()->SetLogy();
          hd[v]["pp"]->Draw("AXIS");
          gr[v]["sub"]->Draw("pe same");
          gr[v]["pp"]->Draw("pe same");
          comp_::makecanvas(fpdf, pa, leg["ppPbPb"], "");

          ymin = comp_::setyminmax_linear({hd[v]["pp_selfnorm"], hd[v]["sub_selfnorm"]});
          fpdf->prepare();
          fpdf->getc()->SetLogy(0);
          hd[v]["pp_selfnorm"]->Draw("AXIS");
          if(ymin < 0) 
            xjjroot::drawline(hd[v]["pp_selfnorm"]->GetXaxis()->GetXmin(), 0, hd[v]["pp_selfnorm"]->GetXaxis()->GetXmax(), 0, kBlack, 2, 2);
          gr[v]["sub_selfnorm"]->Draw("pe same");
          gr[v]["pp_selfnorm"]->Draw("pe same");
          comp_::makecanvas(fpdf, pa, leg["ppPbPb"], "", "Self-normalized");

          comp_::setyminmax_log({hd[v]["pp_selfnorm"], hd[v]["sub_selfnorm"]});
          fpdf->prepare();
          fpdf->getc()->SetLogy();
          hd[v]["pp_selfnorm"]->Draw("AXIS");
          gr[v]["sub_selfnorm"]->Draw("pe same");
          gr[v]["pp_selfnorm"]->Draw("pe same");
          comp_::makecanvas(fpdf, pa, leg["ppPbPb"], "", "Self-normalized");
        }
      if(leg.find("ppMCPbPb") != leg.end())
        {
          float ymin = comp_::setyminmax_linear({hd[v]["pp"], hd[v]["sub"], hd[v]["MC_pp"]});
          fpdf->prepare();
          fpdf->getc()->SetLogy(0);
          hd[v]["pp"]->Draw("AXIS");
          gr[v]["MC_pp"]->Draw("lX0 same");
          if(ymin < 0) 
            xjjroot::drawline(hd[v]["pp"]->GetXaxis()->GetXmin(), 0, hd[v]["pp"]->GetXaxis()->GetXmax(), 0, kBlack, 2, 2);
          gr[v]["sub"]->Draw("pe same");
          gr[v]["pp"]->Draw("pe same");
          comp_::makecanvas(fpdf, pa, leg["ppMCPbPb"], "");

          comp_::setyminmax_log({hd[v]["pp"], hd[v]["sub"], hd[v]["MC_pp"]});
          fpdf->prepare();
          fpdf->getc()->SetLogy();
          hd[v]["pp"]->Draw("AXIS");
          gr[v]["MC_pp"]->Draw("lX0 same");
          gr[v]["sub"]->Draw("pe same");
          gr[v]["pp"]->Draw("pe same");
          comp_::makecanvas(fpdf, pa, leg["ppMCPbPb"], "");

          ymin = comp_::setyminmax_linear({hd[v]["pp_selfnorm"], hd[v]["sub_selfnorm"], hd[v]["MC_pp_selfnorm"]});
          fpdf->prepare();
          fpdf->getc()->SetLogy(0);
          hd[v]["pp_selfnorm"]->Draw("AXIS");
          gr[v]["MC_pp_selfnorm"]->Draw("lX0 same");
          if(ymin < 0) 
            xjjroot::drawline(hd[v]["pp_selfnorm"]->GetXaxis()->GetXmin(), 0, hd[v]["pp_selfnorm"]->GetXaxis()->GetXmax(), 0, kBlack, 2, 2);
          gr[v]["sub_selfnorm"]->Draw("pe same");
          gr[v]["pp_selfnorm"]->Draw("pe same");
          comp_::makecanvas(fpdf, pa, leg["ppMCPbPb"], "", "Self-normalized");

          comp_::setyminmax_log({hd[v]["pp_selfnorm"], hd[v]["sub_selfnorm"], hd[v]["MC_pp_selfnorm"]});
          fpdf->prepare();
          fpdf->getc()->SetLogy();
          hd[v]["pp_selfnorm"]->Draw("AXIS");
          gr[v]["MC_pp_selfnorm"]->Draw("lX0 same");
          gr[v]["sub_selfnorm"]->Draw("pe same");
          gr[v]["pp_selfnorm"]->Draw("pe same");
          comp_::makecanvas(fpdf, pa, leg["ppMCPbPb"], "", "Self-normalized");
        }

      if(leg.find("ppMCPbPb_avg") != leg.end())
        {
          float ymin = comp_::setyminmax_linear({hd[v]["pp"], hd[v]["sub_avg"], hd[v]["MC_pp"]});
          fpdf->prepare();
          fpdf->getc()->SetLogy(0);
          hd[v]["pp"]->Draw("AXIS");
          gr[v]["MC_pp"]->Draw("lX0 same");
          if(ymin < 0) 
            xjjroot::drawline(hd[v]["pp"]->GetXaxis()->GetXmin(), 0, hd[v]["pp"]->GetXaxis()->GetXmax(), 0, kBlack, 2, 2);
          gr[v]["sub_avg"]->Draw("pe same");
          gr[v]["pp"]->Draw("pe same");
          comp_::makecanvas(fpdf, pa, leg["ppMCPbPb_avg"], "");

          comp_::setyminmax_log({hd[v]["pp"], hd[v]["sub_avg"], hd[v]["MC_pp"]});
          fpdf->prepare();
          fpdf->getc()->SetLogy();
          hd[v]["pp"]->Draw("AXIS");
          gr[v]["MC_pp"]->Draw("lX0 same");
          gr[v]["sub_avg"]->Draw("pe same");
          gr[v]["pp"]->Draw("pe same");
          comp_::makecanvas(fpdf, pa, leg["ppMCPbPb_avg"], "");

          ymin = comp_::setyminmax_linear({hd[v]["pp_selfnorm"], hd[v]["sub_avg_selfnorm"], hd[v]["MC_pp_selfnorm"]});
          fpdf->prepare();
          fpdf->getc()->SetLogy(0);
          hd[v]["pp_selfnorm"]->Draw("AXIS");
          gr[v]["MC_pp_selfnorm"]->Draw("lX0 same");
          if(ymin < 0) 
            xjjroot::drawline(hd[v]["pp_selfnorm"]->GetXaxis()->GetXmin(), 0, hd[v]["pp_selfnorm"]->GetXaxis()->GetXmax(), 0, kBlack, 2, 2);
          gr[v]["sub_avg_selfnorm"]->Draw("pe same");
          gr[v]["pp_selfnorm"]->Draw("pe same");
          comp_::makecanvas(fpdf, pa, leg["ppMCPbPb_avg"], "", "Self-normalized");

          comp_::setyminmax_log({hd[v]["pp_selfnorm"], hd[v]["sub_avg_selfnorm"], hd[v]["MC_pp_selfnorm"]});
          fpdf->prepare();
          fpdf->getc()->SetLogy();
          hd[v]["pp_selfnorm"]->Draw("AXIS");
          gr[v]["MC_pp_selfnorm"]->Draw("lX0 same");
          gr[v]["sub_avg_selfnorm"]->Draw("pe same");
          gr[v]["pp_selfnorm"]->Draw("pe same");
          comp_::makecanvas(fpdf, pa, leg["ppMCPbPb_avg"], "", "Self-normalized");
        }
      fpdf->close();
    }

  return 0;
}

void comp_::seth(TH1F* h, float yl, float yh)
{
  xjjroot::sethempty(h, 0, 0.1);
  if(yl!=yh)
    {
      h->SetMinimum(h->GetMinimum()*yl);
      h->SetMaximum(h->GetMaximum()*yh);
    }
  h->GetXaxis()->SetNdivisions(505);
}

int main(int argc, char* argv[])
{
  if(argc==7)
    return comp_drawhist(argv[1], argv[2], argv[3], argv[4], argv[5], argv[6]);
  return 1;
}

void comp_::makecanvas(xjjroot::mypdf* f, Djet::param& pa, TLegend* leg, std::string syst, std::string comment)
{
  pa.drawtex(0.63, 0.85, 0.035, "", 0, -0.2); // 0.23
  leg->Draw();
  xjjroot::drawCMSleft();
  xjjroot::drawCMSright(Form("%s #sqrt{s_{NN}} = 5.02 TeV", syst.c_str()));
  xjjroot::drawtex(0.63, 0.60, comment.c_str(), 0.035, 13, 62);
  // xjjroot::drawcomment(comp_::outputdir, "lb");
  f->write();
}

float comp_::setyminmax_linear(std::vector<TH1F*> h)
{
  double ymin = 1.e+10, ymax = -1.e+10;
  for(auto& hh : h)
    {
      ymin = std::min(ymin, xjjana::gethminimum(hh));
      ymax = std::max(ymax, xjjana::gethmaximum(hh));
    }
  if(ymin < 0) ymin = ymin * 1.6;
  else ymin = 0;
  ymax = ymax * 1.6;
  for(auto& hh : h)
    {
      hh->SetMinimum(ymin);
      hh->SetMaximum(ymax);
    }
  return ymin;
}

float comp_::setyminmax_log(std::vector<TH1F*> h)
{
  double ymin = 1.e+10, ymax = -1.e+10;
  for(auto& hh : h)
    {
      if(xjjana::gethminimum(hh) > 0) ymin = std::min(ymin, xjjana::gethminimum(hh));
      ymax = std::max(ymax, xjjana::gethmaximum(hh));
    }
  ymin = ymin * 0.1;
  ymax = ymax * 5;
  for(auto& hh : h)
    {
      hh->SetMinimum(ymin);
      hh->SetMaximum(ymax);
    }
  return ymin;
}
