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
  // ratio
  if(hvar["HMCMatchedGenptRecophi-original"]) std::cout<<"\e[32m"<<"HMCMatchedGenptRecophi-original"<<"\e[0m"<<std::endl;
  else std::cout<<"\e[31m"<<"HMCMatchedGenptRecophi-original"<<"\e[0m"<<std::endl;
  hvar["hratio_GenptRecophi_RecoptRecophi-original"] = (TH1D*)hvar["HMCMatchedGenptRecophi-original"]->Clone("hratio_GenptRecophi_RecoptRecophi-original");
  hvar["hratio_GenptRecophi_RecoptRecophi-original"]->Divide(hvar["HMCMatched-original"]);
  hvar["hratio_GenptRecophi_RecoptRecophi-original"]->GetYaxis()->SetTitle("(Gen p_{T} Reco #phi,#eta) / (Reco p_{T} Reco #phi,#eta)");
  // correct jer
  if(hvar["HDataReco-original"]) std::cout<<"\e[32m"<<"HDataReco-original"<<"\e[0m"<<std::endl;
  else std::cout<<"\e[31m"<<"HDataReco-original"<<"\e[0m"<<std::endl;
  if(hvar["hratio_GenptRecophi_RecoptRecophi-original"]) std::cout<<"\e[32m"<<"hratio_GenptRecophi_RecoptRecophi-original"<<"\e[0m"<<std::endl;
  else std::cout<<"\e[31m"<<"hratio_GenptRecophi_RecoptRecophi-original"<<"\e[0m"<<std::endl;
  hvar["HDataReco_nojercorr-original"] = (TH1D*)hvar["HDataReco-original"]->Clone("HDataReco_nojercorr-original");
  hvar["HDataReco-original"]->Multiply(hvar["hratio_GenptRecophi_RecoptRecophi-original"]);
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
      double norm = 0;
      for(int k=0; k<hvar[t+"-original"]->GetXaxis()->GetNbins(); k++)
        {
          double bin_width = vb.width(k);
          if(var=="dr") bin_width = vb.area(k);
          norm += bin_width*hvar[t+"-original"]->GetBinContent(k+1);
        }
      if(norm) hvar[t+"_norm-original"]->Scale(1./norm);
      hvar[t+"_norm-original"]->GetYaxis()->SetTitle(Form("#frac{1}{N_{jD}} %s", Djet::varytex[var].c_str()));
    }
  if(hvar["HMCMatchedGenptRecophi_norm-original"]) std::cout<<"\e[32m"<<"HMCMatchedGenptRecophi_norm-original"<<"\e[0m"<<std::endl;
  else std::cout<<"\e[31m"<<"HMCMatchedGenptRecophi_norm-original"<<"\e[0m"<<std::endl;
  hvar["hratio_GenptRecophi_RecoptRecophi_norm-original"] = (TH1D*)hvar["HMCMatchedGenptRecophi_norm-original"]->Clone("hratio_GenptRecophi_RecoptRecophi_norm-original");
  hvar["hratio_GenptRecophi_RecoptRecophi_norm-original"]->Divide(hvar["HMCMatched_norm-original"]);
  hvar["hratio_GenptRecophi_RecoptRecophi_norm-original"]->GetYaxis()->SetTitle("(Gen p_{T} Reco #phi,#eta) / (Reco p_{T} Reco #phi,#eta)");
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
