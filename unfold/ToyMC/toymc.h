#include <TH1D.h>
#include <TH2D.h>
#include <string>

namespace toymc
{
  TH1D* makehreco(TH1D* htruth, TH2D* hresponse, std::string name)
  {
    auto hres = (TH2D*)hresponse->Clone("hres");
    xjjana::normTH2(hres, "y");
    TH1D* hreco = (TH1D*)htruth->Clone(name.c_str());
    hreco->Reset("ICESM");
    int nbin = htruth->GetXaxis()->GetNbins();
    for(int ix=0; ix<nbin; ix++)
      {
        double result = 0;
        for(int iy=0; iy<nbin; iy++)
          {
            double val = hres->GetBinContent(ix+1, iy+1) * htruth->GetBinContent(iy+1);
            result += val;
          }
        hreco->SetBinContent(ix+1, result);
        hreco->SetBinError(ix+1, std::sqrt(result));
      }
    delete hres;
    return hreco;
  }

  int getndis(TH1D* hdata)
  {
    double errdata = 0;
    double relerrdata = hdata->IntegralAndError(1, hdata->GetXaxis()->GetNbins(), errdata);
    if(errdata<=0) { std::cout<<__FUNCTION__<<"\e[31;1merror:\e[0m: bad errdata ("<<errdata<<", "<<relerrdata<<")"<<std::endl; return 0; }
    relerrdata /= errdata;
    int ndis = std::ceil(relerrdata*relerrdata);
    return ndis;
  }

  TH1D* sample(TH1D* hreco, TH1D* hdata, std::string name)
  {
    int ndis = getndis(hdata);
    TH1D* htoy = (TH1D*)hreco->Clone(name.c_str());
    htoy->Reset("ICESM");
    htoy->FillRandom(hreco, ndis);
    return htoy;
  }

}
