#include <TFile.h>
#include <TH1F.h>
#include <iostream>
#include <iomanip>

#include "para.h"
#include "bins.h"

#include "xjjcuti.h"
#include "xjjrootuti.h"
#include "xjjmypdf.h"
#include "dfitter.h"

int djana_fithist(std::string inputname, std::string outsubdir)
{
  TFile* inf = TFile::Open(inputname.c_str());
  Djet::param pa(inf);
  std::vector<std::string> type = {"fitweigh", "unweight"};

  // binning
  std::map<std::string, xjjana::bins<float>> vb;
  vb["dphi"] = xjjana::bins<float>(pa.ishi()?Djet::bins_aa["dphi"]:Djet::bins_pp["dphi"]);
  vb["dr"] = xjjana::bins<float>(pa.ishi()?Djet::bins_aa["dr"]:Djet::bins_pp["dr"]);

  xjjc::prt_divider();

  // get hmass, heff
  std::map<std::string, std::vector<TH1F*>> hmass;
  std::map<std::string, TH1F*> hmass_incl, hd, heff_sigreg, heffnorm_sigreg, heff_sideband, heffnorm_sideband, heff, heffnorm, heff_nosub, heffnorm_nosub;
  for(auto& v : Djet::var) // dphi, dr
    {
      for(auto& t : type) // fitweight, unweight
        {
          hmass_incl[v+"_"+t] = xjjroot::gethist<TH1F>(inf, Form("hmass_incl_%s_%s", v.c_str(), t.c_str()));
          for(int k=0; k<vb[v].n(); k++)
            {
              TH1F* h = xjjroot::gethist<TH1F>(inf, Form("hmass_%s_%s_%d", v.c_str(), t.c_str(), k));
              hmass[v+"_"+t].push_back(h);
            }
          hd[v+"_"+t] = new TH1F(Form("h%s_%s", v.c_str(), t.c_str()), Form(";%s;%s", Djet::vartex[v].c_str(), Djet::varytex[v].c_str()), vb[v].n(), vb[v].v().data());
        }
      hd[v+"_effcorr"] = new TH1F(Form("h%s_effcorr", v.c_str()), Form(";%s;%s", Djet::vartex[v].c_str(), Djet::varytex[v].c_str()), vb[v].n(), vb[v].v().data());
    }
  type.push_back("effcorr");

  for(auto& v : Djet::var) // dphi, dr
    {
      heff_sigreg[v] = xjjroot::gethist<TH1F>(inf, Form("heff_sigreg_%s", v.c_str()));
      heffnorm_sigreg[v] = xjjroot::gethist<TH1F>(inf, Form("heffnorm_sigreg_%s", v.c_str()));
      heff_sideband[v] = xjjroot::gethist<TH1F>(inf, Form("heff_sideband_%s", v.c_str()));
      heff_sideband[v]->Scale(pdg::DZERO_SCALE);
      heffnorm_sideband[v] = xjjroot::gethist<TH1F>(inf, Form("heffnorm_sideband_%s", v.c_str()));
      heffnorm_sideband[v]->Scale(pdg::DZERO_SCALE);

      heff[v] = (TH1F*)heff_sigreg[v]->Clone(Form("heff_%s", v.c_str()));
      heff[v]->Add(heff_sideband[v], -1);
      heffnorm[v] = (TH1F*)heffnorm_sigreg[v]->Clone(Form("heffnorm_%s", v.c_str()));
      heffnorm[v]->Add(heffnorm_sideband[v], -1);
      heff_nosub[v] = (TH1F*)heff_sigreg[v]->Clone(Form("heff_nosub_%s", v.c_str()));
      heffnorm_nosub[v] = (TH1F*)heffnorm_sigreg[v]->Clone(Form("heffnorm_nosub_%s", v.c_str()));
      for(int i=0; i<vb[v].n(); i++)
        {
          if(heffnorm[v]->GetBinContent(i+1)==0)
            {
              heff[v]->SetBinContent(i+1, 0);
              heff[v]->SetBinError(i+1, 0);
            }
          else
            {
              heff[v]->SetBinContent(i+1, heff[v]->GetBinContent(i+1)/heffnorm[v]->GetBinContent(i+1));
              heff[v]->SetBinError(i+1, heff[v]->GetBinError(i+1)/heffnorm[v]->GetBinContent(i+1));
            }
          if(heffnorm_nosub[v]->GetBinContent(i+1)==0)
            {
              heff_nosub[v]->SetBinContent(i+1, 0);
              heff_nosub[v]->SetBinError(i+1, 0);
            }
          else
            {
              heff_nosub[v]->SetBinContent(i+1, heff_nosub[v]->GetBinContent(i+1)/heffnorm_nosub[v]->GetBinContent(i+1));
              heff_nosub[v]->SetBinError(i+1, heff_nosub[v]->GetBinError(i+1)/heffnorm_nosub[v]->GetBinContent(i+1));
            }
        }
    }

  // get njet
  TH1F* hnjet = xjjroot::gethist<TH1F>(inf, "hnjet");
  float njet = hnjet->GetBinContent(1);
  // get hmassmc
  TFile* inftpl = TFile::Open("masstpl_PbPb.root");
  TH1F* hmassmc_signal = xjjroot::gethist<TH1F>(inftpl, "hHistoRMassSignal_pt_0_dr_0");
  TH1F* hmassmc_swapped = xjjroot::gethist<TH1F>(inftpl, "hHistoRMassSwapped_pt_0_dr_0");

  xjjc::prt_divider();

  xjjroot::setgstyle();
  // unweight
  xjjroot::dfitter df("YC");
  for(auto& v : Djet::var) // dphi, dr
    {
      // std::string fitoutput = "plots/" + outsubdir + "_" + pa.tag() + "/idx_" + v + "/cmass";
      xjjroot::mypdf* fpdf = new xjjroot::mypdf("plots/" + outsubdir + "_" + pa.tag() + "/idx_" + v + "_cmass_unweight.pdf", "c", 600, 600);
      fpdf->prepare();
      df.fit(fpdf->getc(), hmass_incl[v+"_unweight"], hmassmc_signal, hmassmc_swapped, pa.tag("ishi").c_str(),
             std::vector<TString>({pa.tag("pt").c_str(), pa.tag("y").c_str()}));
      fpdf->write();
      for(int k=0; k<vb[v].n(); k++)
        {
          float bin_width = vb[v].width(k); // dphi
          if(v=="dr") bin_width = vb[v].area(k);

          fpdf->prepare();
          df.fit(fpdf->getc(), hmass[v+"_unweight"][k], hmassmc_signal, hmassmc_swapped, pa.tag("ishi").c_str(),
                 std::vector<TString>({pa.tag("pt").c_str(), pa.tag("y").c_str(), vb[v].tag(k, Djet::vartex[v]).c_str()}));
          fpdf->write();
          hd[v+"_unweight"]->SetBinContent(k+1, df.GetY()/bin_width);
          hd[v+"_unweight"]->SetBinError(k+1, df.GetYE()/bin_width);
          hd[v+"_effcorr"]->SetBinContent(k+1, df.GetY()/bin_width*heff[v]->GetBinContent(k+1));
          hd[v+"_effcorr"]->SetBinError(k+1, df.GetYE()/bin_width*heff[v]->GetBinContent(k+1));
        }
      fpdf->close();
    }

  // weight
  xjjroot::dfitter dfw("YCW");
  for(auto& v : Djet::var) // dphi, dr
    {
      xjjroot::mypdf* fpdfw = new xjjroot::mypdf("plots/" + outsubdir + "_" + pa.tag() + "/idx_" + v + "_cmass_fitweight.pdf", "cw", 600, 600);

      fpdfw->prepare();
      dfw.fit(fpdfw->getc(), hmass_incl[v+"_fitweigh"], hmassmc_signal, hmassmc_swapped, pa.tag("ishi").c_str(), 
              std::vector<TString>({pa.tag("pt").c_str(), pa.tag("y").c_str()}));
      fpdfw->write();
      for(int k=0; k<vb[v].n(); k++)
        {
          float bin_width = vb[v].width(k); // dphi
          if(v=="dr") bin_width = vb[v].area(k);

          fpdfw->prepare();
          dfw.fit(fpdfw->getc(), hmass[v+"_fitweigh"][k], hmassmc_signal, hmassmc_swapped, pa.tag("ishi").c_str(),
                  std::vector<TString>({pa.tag("pt").c_str(), pa.tag("y").c_str(), vb[v].tag(k, Djet::vartex[v]).c_str()}));
          fpdfw->write();
          hd[v+"_fitweigh"]->SetBinContent(k+1, dfw.GetY()/bin_width);
          hd[v+"_fitweigh"]->SetBinError(k+1, dfw.GetYE()/bin_width);
        }
      fpdfw->close();
    }

  for(auto& v : Djet::var) // dphi, dr
    for(auto& t : type)
      hd[v+"_"+t]->SetTitle(Form("%f", njet));

  std::string outputname = "rootfiles/" + outsubdir + "_" + pa.tag() + "/fithist.root";
  xjjroot::mkdir(outputname);
  TFile* outf = new TFile(outputname.c_str(), "recreate");
  for(auto& h : hd) xjjroot::writehist(h.second, 10);
  for(auto& h : heff) xjjroot::writehist(h.second, 10);
  for(auto& h : heff_nosub) xjjroot::writehist(h.second, 10);
  pa.write();
  outf->Close();

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc==3)
    return djana_fithist(argv[1], argv[2]);
  return 1;
}
