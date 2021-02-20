#include <vector>
#include <string>
#include "xjjcuti.h"

namespace eff
{
  class effbins
  {
  public:
    effbins(bool ishi) : fishi(ishi) { fcentbins = fishi?fcentbins_PbPb:fcentbins_pp; }
    effbins(std::string inputfile, bool ishi);
    float geteff(float pt, float absy, int hiBin);
    int npt() { return fptbins.size()-1; }
    int ny() { return fybins.size()-1; }
    int ncent() { return fcentbins.size()-1; }
    int nycent() { return (fybins.size()-1)*(fcentbins.size()-1); }
    float* ptbins() { return fptbins.data(); }
    int icent(int hiBin) { return xjjc::findibin(fcentbins, (float)(hiBin/2.)); }
    int ipt(float pt);
    int iy(float y) { return xjjc::findibin(fybins, y); }
    int iycent(float y, int hiBin);
    std::vector<std::string> label(int iycent);
    std::vector<int> index(int iycent);
  private:
    bool fishi;
    std::vector<TH1F*> fheff;
    std::vector<float> fcentbins;
    std::vector<float> fcentbins_PbPb = {0, 30, 50, 90};
    std::vector<float> fcentbins_pp = {-1, 0};
    std::vector<float> fptbins = {2, 2.5, 3., 3.5, 4., 4.5, 5., 5.5, 6., 6.5, 7., 8., 9., 10., 12., 14., 16., 20., 30.};
    std::vector<float> fybins = {0., 1.2};
    std::string ftpt(int ipt);
    std::string fty(int iy);
    std::string ftcent(int icent);
  };
}

eff::effbins::effbins(std::string inputfile, bool ishi) : fishi(ishi)
{
  TFile* inf = TFile::Open(inputfile.c_str());
  fcentbins = fishi?fcentbins_PbPb:fcentbins_pp;
  for(int k=0; k<nycent(); k++)
    {
      TH1F* h = (TH1F*)inf->Get(Form("heff_%d", k));
      fheff.push_back(h);
    }
}

float eff::effbins::geteff(float pt, float absy, int hiBin)
{
  // std::cout<<pt<<" "<<absy<<" "<<hiBin<<" "<<ipt(pt)<<" "<<iycent(absy, hiBin)<<std::endl;
  return fheff[iycent(absy, hiBin)]->GetBinContent(ipt(pt)+1);
}

int eff::effbins::ipt(float pt)
{
  if(pt >= fptbins.back()) { return fptbins.size()-1; }
  if(pt < fptbins.front()) { return 0; }
  return xjjc::findibin(fptbins, pt);
}

int eff::effbins::iycent(float y, int hiBin)
{
  int iiy = iy(y), iicent = icent(hiBin), nncent = ncent();
  // std::cout<<" ("<<iiy<<" "<<iicent<<")";
  if(iicent < 0 || iiy < 0) return -1;
  return iiy*nncent+iicent;
}

std::vector<int> eff::effbins::index(int iycent)
{
  std::vector<int> idx(2);
  int nncent = ncent();
  idx[0] = std::floor(iycent/(float)nncent);  
  idx[1] = iycent%nncent;
  return idx;
}

std::vector<std::string> eff::effbins::label(int iycent)
{
  int iiy = index(iycent)[0];
  int iicent = index(iycent)[1];
  std::vector<std::string> t(2);
  t[0] = Form("%s < |y| < %s", 
              xjjc::number_remove_zero(fybins[iiy]).c_str(), 
              xjjc::number_remove_zero(fybins[iiy+1]).c_str());
  t[1] = fishi?Form("Cent. %.0f-%.0f%s", fcentbins[iicent], fcentbins[iicent+1], "%"):"";
  return t;
}
