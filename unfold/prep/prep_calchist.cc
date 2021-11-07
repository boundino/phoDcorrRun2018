#include <TFile.h>
#include <TH2D.h>
#include <TH1D.h>
#include <TKey.h>
// #include <TClass.h>
#include <TObjString.h>
#include <string>
#include <map>
#include <vector>

#include "para.h"
#include "bins.h"

#include "xjjrootuti.h"
#include "xjjanauti.h"
#include "xjjcuti.h"
#include "xjjmypdf.h"

int prep_calchist(std::string inputname, std::string outputdir, std::string var)
{
  TFile* inf = TFile::Open(inputname.c_str());
  Djet::param* pa = new Djet::param(inf, "");;
  std::map<std::string, TH1D*> hvar;
  TH2D* HResponse = (TH2D*)inf->Get("HResponse");
  TKey* key; TIter nextkey(inf->GetListOfKeys());
  while((key = (TKey*)nextkey()))
    {
      std::string classname(key->GetClassName()), keyname(key->GetName());
      if(classname != "TH1D") continue;
      std::cout<<"  => reading: "<<keyname<<std::endl;
      hvar[keyname] = (TH1D*)inf->Get(keyname.c_str());
    }
  xjjana::bins<double> vb(xjjana::gethXaxis(hvar["HDataReco-original"]));
  // remove bandwidth scale for data
  for(int k=0; k<vb.n(); k++)
    {
      double bin_width = vb.width(k);
      if(var=="dr") bin_width = vb.area(k);
      hvar["HDataReco-original"]->SetBinContent(k+1, hvar["HDataReco-original"]->GetBinContent(k+1) * bin_width);
      hvar["HDataReco-original"]->SetBinError(k+1, hvar["HDataReco-original"]->GetBinError(k+1) * bin_width);
    }
  //
  std::vector<std::string> names;
  for(auto& h : hvar)
    {
      if(!xjjc::str_contains(h.first, "-original")) continue;
      auto tag = xjjc::str_replaceall(h.first, "-original", "");
      names.push_back(tag);
    }
  // norm
  for(auto& t : names)
    {
      hvar[t+"_norm-original"] = (TH1D*)hvar[t+"-original"]->Clone(Form("%s_norm-original", t.c_str()));
      double norm = hvar[t+"-original"]->Integral();
      // double norm = 0;
      // for(int k=0; k<hvar[t+"-original"]->GetXaxis()->GetNbins(); k++)
      //   {
      //     double bin_width = vb.width(k);
      //     if(var=="dr") bin_width = vb.area(k);
      //     norm += bin_width*hvar[t+"-original"]->GetBinContent(k+1);
      //   }
      if(norm) hvar[t+"_norm-original"]->Scale(1./norm);
      hvar[t+"-original"]->GetYaxis()->SetTitle(Form("#frac{1}{N_{jet}} %s", Djet::varytex[var].c_str()));
      hvar[t+"_norm-original"]->GetYaxis()->SetTitle(Form("#frac{1}{N_{jD}} %s", Djet::varytex[var].c_str()));
    }
  // ratio
  hvar["hratio_GenptRecophi_RecoptRecophi-original"] = (TH1D*)hvar["HMCMatchedGenptRecophi-original"]->Clone("hratio_GenptRecophi_RecoptRecophi-original");
  hvar["hratio_GenptRecophi_RecoptRecophi-original"]->Divide(hvar["HMCMatched-original"]);
  hvar["hratio_GenptRecophi_RecoptRecophi-original"]->GetYaxis()->SetTitle("(Gen p_{T} Reco #phi,#eta) / (Reco p_{T} Reco #phi,#eta)");
  hvar["hratio_GenptRecophi_RecoptRecophi_norm-original"] = (TH1D*)hvar["HMCMatchedGenptRecophi_norm-original"]->Clone("hratio_GenptRecophi_RecoptRecophi_norm-original");
  hvar["hratio_GenptRecophi_RecoptRecophi_norm-original"]->Divide(hvar["HMCMatched_norm-original"]);
  hvar["hratio_GenptRecophi_RecoptRecophi_norm-original"]->GetYaxis()->SetTitle("(Gen p_{T} Reco #phi,#eta) / (Reco p_{T} Reco #phi,#eta)");
  names.push_back("hratio_GenptRecophi_RecoptRecophi");
  hvar["hratio_GenptGenphi_GenptRecophi-original"] = (TH1D*)hvar["HMCMatchedGenptGenphi-original"]->Clone("hratio_GenptGenphi_GenptRecophi-original");
  hvar["hratio_GenptGenphi_GenptRecophi-original"]->Divide(hvar["HMCMatched-original"]);
  hvar["hratio_GenptGenphi_GenptRecophi-original"]->GetYaxis()->SetTitle("(Gen p_{T} Gen #phi,#eta) / (Gen p_{T} Reco #phi,#eta)");
  hvar["hratio_GenptGenphi_GenptRecophi_norm-original"] = (TH1D*)hvar["HMCMatchedGenptGenphi_norm-original"]->Clone("hratio_GenptGenphi_GenptRecophi_norm-original");
  hvar["hratio_GenptGenphi_GenptRecophi_norm-original"]->Divide(hvar["HMCMatched_norm-original"]);
  hvar["hratio_GenptGenphi_GenptRecophi_norm-original"]->GetYaxis()->SetTitle("(Gen p_{T} Gen #phi,#eta) / (Gen p_{T} Reco #phi,#eta)");
  names.push_back("hratio_GenptGenphi_GenptRecophi");
  // correct jer
  hvar["HDataReco_nojercorr-original"] = (TH1D*)hvar["HDataReco-original"]->Clone("HDataReco_nojercorr-original");
  hvar["HDataReco-original"]->Multiply(hvar["hratio_GenptRecophi_RecoptRecophi-original"]);
  hvar["HDataReco_nojercorr_norm-original"] = (TH1D*)hvar["HDataReco_norm-original"]->Clone("HDataReco_nojercorr_norm-original");
  hvar["HDataReco_norm-original"]->Multiply(hvar["hratio_GenptRecophi_RecoptRecophi_norm-original"]);
  names.push_back("HDataReco_nojercorr");

  // remove -original
  for(auto& t : names)
    {
      hvar[t] = (TH1D*)xjjana::changebin(hvar[t+"-original"], 0, vb.n(), t);
      hvar[t+"_norm"] = (TH1D*)xjjana::changebin(hvar[t+"_norm-original"], 0, vb.n(), t+"_norm");
      hvar[t]->GetXaxis()->SetTitle(Form("%s bin index", hvar[t]->GetXaxis()->GetTitle()));
      hvar[t+"_norm"]->GetXaxis()->SetTitle(Form("%s bin index", hvar[t+"_norm"]->GetXaxis()->GetTitle()));
    }
  
  std::string outputname = "rootfiles/"+outputdir+".root";
  xjjroot::mkdir(outputname);
  TFile* outf = new TFile(outputname.c_str(), "recreate");
  for(auto& h : hvar) xjjroot::writehist(h.second);
  xjjroot::writehist(HResponse);
  std::vector<std::string> nameds = {
    "DataEventCount", "MCEventCount",
    "DataBaselineEventCount", "MCBaselineEventCount",
    "DataAllEventCount", "MCAllEventCount"
  };
  for(auto& t : nameds)
    {
      TNamed* tn = new TNamed(t.c_str(), "");
      tn->Write();
    }
  pa->write();
  outf->Close();

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc==4)
    return prep_calchist(argv[1], argv[2], argv[3]);
  return 1;
}
