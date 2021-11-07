#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TRandom3.h>
#include <TGraph.h>

#include "RooUnfold.h"
#include "RooUnfoldInvert.h"
#include "RooUnfoldBayes.h"
#include "RooUnfoldSvd.h"
#include "TMatrixD.h"

#include <map>
#include <vector>
#include <iostream>

#include "xjjrootuti.h"
#include "xjjanauti.h"
#include "toymc.h"

RooUnfoldResponse *FillResponse(const TH2 *HResponse);
int toymc_unfold(std::string inputname, std::string output, int ntoy = 10)
{
  TFile* inf = TFile::Open(inputname.c_str());
  TH2D* HResponse = (TH2D*)inf->Get("HResponse");
  RooUnfoldResponse *Response = FillResponse(HResponse);
  // std::vector<int> Iterations = {10};
  std::vector<int> Iterations{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62, 64, 66, 68, 70, 80, 90, 100, 125, 150, 200};
  std::vector<double> double_Iterations; for(auto& k : Iterations) { double_Iterations.push_back((double)k); }
  std::map<std::string, TH1D*> h;
  for(auto& t : {"HDataReco", "HMCGen", "HMCRecoMake", "HMCMatchedGenptRecophi"})
    h[t] = (TH1D*)inf->Get(t);
  std::vector<TH1D*> htoy(ntoy, 0);
  for(int i=0; i<ntoy; i++)
    htoy[i] = (TH1D*)inf->Get(Form("HMCToy_%d", i));
  int nbin = h["HMCGen"]->GetXaxis()->GetNbins(), nIter = Iterations.size();

  // scan toy
  std::vector<double> vchi2_toy(nIter, 0);
  for(int k=0; k<nIter; k++)
    {
      RooUnfoldBayes BayesUnfold(Response, htoy[0], Iterations[k]);
      BayesUnfold.SetVerbose(0);
      auto hreco = (TH1D*)(BayesUnfold.Hreco()->Clone("hreco"));
      double chi2val = hreco->Chi2Test(h["HMCGen"], "WW CHI2");
      delete hreco;
      // double chi2val = BayesUnfold.Chi2(h["HMCGen"]);
      vchi2_toy[k] = chi2val;
      std::cout<<Iterations[k]<<" : "<<vchi2_toy[k]<<", "<<BayesUnfold.Chi2(h["HMCGen"])<<std::endl;
    }
  TGraph* gr_chi2_toy = new TGraph(nIter, double_Iterations.data(), vchi2_toy.data()); gr_chi2_toy->SetName("gr_chi2_toy");
  const int KITER = 12;

  // return 3;

  // toy
  std::vector<TH1D*> htoy_Bayes(ntoy); 
  for(int i=0; i<ntoy; i++)
    {
      htoy[i] = (TH1D*)inf->Get(Form("HMCToy_%d", i));
      RooUnfoldBayes BayesUnfold(Response, htoy[i], KITER);
      BayesUnfold.SetVerbose(0);
      htoy_Bayes[i] = (TH1D*)(BayesUnfold.Hreco()->Clone(Form("HMCToy_UnfoldedBayes_%d_%d", i, KITER)));
    }

  // pull
  std::vector<TH1D*> hpull(nbin);
  for(int j=0; j<nbin; j++)
    {
      hpull[j] = new TH1D(Form("hpull_%d", j), ";Pull;", 50, -5, 5);
      for(int i=0; i<ntoy; i++)
        {
          double pullval = (htoy_Bayes[i]->GetBinContent(j+1) - h["HMCGen"]->GetBinContent(j+1)) / htoy_Bayes[i]->GetBinError(j+1);
          hpull[j]->Fill(pullval);
        }
      hpull[j]->Scale(1./ntoy);
    }

  // data
  std::vector<TH1D*> HDataReco_Bayes(nIter, 0);
  for(int k=0; k<nIter; k++)
    {
      RooUnfoldBayes BayesUnfold(Response, h["HDataReco"], Iterations[k]);
      BayesUnfold.SetVerbose(0);
      HDataReco_Bayes[k] = (TH1D*)(BayesUnfold.Hreco()->Clone(Form("HDataReco_Bayes_%d", Iterations[k])));
    }

  // pseudo data
  auto hdatatruth = (TH1D*)HDataReco_Bayes[KITER]->Clone("hdatatruth");
  hdatatruth->Scale(toymc::getndis(h["HDataReco"]) / hdatatruth->Integral());
  auto hdatatruthReco = toymc::makehreco(hdatatruth, HResponse, "hdatatruthReco");
  auto hdatatruthRecoSample = toymc::sample(hdatatruthReco, h["HDataReco"], "hdatatruthRecoSample");
  std::vector<double> vchi2_data(nIter, 0);
  std::vector<TH1D*> HPseudoDataReco_Bayes(nIter, 0);
  for(int k=0; k<nIter; k++)
    {
      RooUnfoldBayes BayesUnfold(Response, hdatatruthRecoSample, Iterations[k]);
      BayesUnfold.SetVerbose(0);
      HPseudoDataReco_Bayes[k] = (TH1D*)(BayesUnfold.Hreco()->Clone(Form("HPseudoDataReco_Bayes_%d", Iterations[k])));
      double chi2val = HPseudoDataReco_Bayes[k]->Chi2Test(hdatatruth, "WW CHI2");
      vchi2_data[k] = chi2val;
      // std::cout<<Iterations[k]<<": "<<vchi2_data[k]<<", "<<BayesUnfold.Chi2(hdatatruth)<<std::endl;
      std::cout<<Iterations[k]<<": "<<vchi2_data[k]<<std::endl;
    }
  TGraph* gr_chi2_data = new TGraph(nIter, double_Iterations.data(), vchi2_data.data()); gr_chi2_data->SetName("gr_chi2_data");

  std::string outputname = "ToyMC/"+output+".root";
  xjjroot::mkdir(outputname);
  TFile* outf = new TFile(outputname.c_str(), "recreate");
  for(auto& hh : h) hh.second->Write();
  for(auto& hh : htoy) hh->Write();
  for(auto& hh : htoy_Bayes) hh->Write();
  for(auto& hh : HDataReco_Bayes) hh->Write();
  for(auto& hh : HPseudoDataReco_Bayes) hh->Write();
  for(auto& hh : hpull) hh->Write();
  hdatatruth->Write();
  hdatatruthReco->Write();
  hdatatruthRecoSample->Write();
  gr_chi2_toy->Write();
  gr_chi2_data->Write();
  HResponse->Write();
  outf->Close();

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc==3)
    return toymc_unfold(argv[1], argv[2]);
  return 1;
}


RooUnfoldResponse *FillResponse(const TH2 *HResponse)
{
  int NX = HResponse->GetNbinsX();
  int NY = HResponse->GetNbinsY();
  double XMin = HResponse->GetXaxis()->GetBinLowEdge(1);
  double YMin = HResponse->GetYaxis()->GetBinLowEdge(1);
  double XMax = HResponse->GetXaxis()->GetBinUpEdge(NX);
  double YMax = HResponse->GetYaxis()->GetBinUpEdge(NY);

  RooUnfoldResponse *Response = new RooUnfoldResponse(NX, XMin, XMax, NY, YMin, YMax);

  for(int i = 1; i <= HResponse->GetNbinsX(); i++)
    {
      for(int j = 1; j <= HResponse->GetNbinsY(); j++)
        {
          double x = HResponse->GetXaxis()->GetBinCenter(i);
          double y = HResponse->GetYaxis()->GetBinCenter(j);

          Response->Fill(x, y, HResponse->GetBinContent(i, j));
          // for(int k = 0; k < HResponse->GetBinContent(i, j); k++)
          //   Response->Fill(x, y);
        }
    }

  return Response;
}

