#include <TF1.h>
#include "xjjrootuti.h"

namespace Djet
{
  class jetreso
  {
  public:
    jetreso(int ishi);
    ~jetreso() { for(auto& ff : {f_, f1_, f2_, f3_}) delete ff; }
    float par3();
    float par4(float pt=0);
    TF1* f() { return f_; }
    TF1* freso() { return freso_; }
    void drawfcomp(float xmin, float xmax);
    float sigma_phi(int icent, float pt);
    float sigma_eta(int icent, float pt);
    
  private:
    int ishi_;
    TF1 *f_, *f1_, *f2_, *f3_, *freso_;
    std::vector<std::vector<float>> par_reso_eta_pp_ = {{0, 0.1893, 0}};
    std::vector<std::vector<float>> par_reso_phi_pp_ = {{0, 0.2298, 0}};
    std::vector<std::vector<float>> par_reso_eta_PbPb_ = {{0.0317, 0.1662, 3.8142},
                                                          {0.0303, 0.0755, 1.9867}};
    std::vector<std::vector<float>> par_reso_phi_PbPb_ = {{0.0272, 0.2905, 3.7818},
                                                          {0.0276, 0.2151, 1.8010}};
  };
}

Djet::jetreso::jetreso(int ishi) : ishi_(ishi)
{
  f1_ = new TF1("f1", "[0]*[3]*TMath::Gaus(x,[1],[2])/(sqrt(2*3.1415926)*[2])");
  f2_ = new TF1("f2", "[0]*(1-[3])*[4]*TMath::Gaus(x,[1],[2]*0.5)/(sqrt(2*3.1415926)*[2]*0.5)");
  f3_ = new TF1("f3", "[0]*(1-[3])*(1-[4])*TMath::Gaus(x,[1],[2]*0.2)/(sqrt(2*3.1415926)*[2]*0.2)");
  f_ = new TF1("f", Form("%s+%s+%s", f1_->GetExpFormula().Data(), f2_->GetExpFormula().Data(), f3_->GetExpFormula().Data()));
  freso_ = new TF1("freso", "sqrt([0]*[0]+[1]*[1]/x+[2]*[2]/(x*x))");
}

float Djet::jetreso::par3()
{
  if(ishi_) return 0.03;
  else return 0.2;
}

float Djet::jetreso::par4(float pt)
{
  if(!ishi_) return 0.8;
  float slope = (1-0.92)/(245-75);
  return (pt-75)*slope+0.92;
}

void Djet::jetreso::drawfcomp(float xmin, float xmax)
{
  f1_->SetParameters(f_->GetParameter(0), f_->GetParameter(1), f_->GetParameter(2), f_->GetParameter(3));
  f2_->SetParameters(f_->GetParameter(0), f_->GetParameter(1), f_->GetParameter(2), f_->GetParameter(3), f_->GetParameter(4));
  f3_->SetParameters(f_->GetParameter(0), f_->GetParameter(1), f_->GetParameter(2), f_->GetParameter(3), f_->GetParameter(4));

  for(auto& ff : {f_, f1_, f2_, f3_})
    ff->SetRange(xmin, xmax);

  xjjroot::setlinestyle(f1_, f_->GetLineColor(), 7, f_->GetLineWidth());
  xjjroot::setlinestyle(f2_, f_->GetLineColor(), 7, f_->GetLineWidth());
  xjjroot::setlinestyle(f3_, f_->GetLineColor(), 7, f_->GetLineWidth());

  f1_->Draw("same");
  f2_->Draw("same");
  f3_->Draw("same");
}

float Djet::jetreso::sigma_phi(int icent, float pt)
{
  std::vector<std::vector<float>> par_reso = ishi_?par_reso_phi_PbPb_:par_reso_phi_pp_;
  TF1* ftemp = new TF1("ftemp", freso_->GetExpFormula().Data());
  ftemp->SetParameters(par_reso[icent][0], par_reso[icent][1], par_reso[icent][2]);
  return ftemp->Eval(pt);
}

float Djet::jetreso::sigma_eta(int icent, float pt)
{
  std::vector<std::vector<float>> par_reso = ishi_?par_reso_eta_PbPb_:par_reso_eta_pp_;
  TF1* ftemp = new TF1("ftemp", freso_->GetExpFormula().Data());
  ftemp->SetParameters(par_reso[icent][0], par_reso[icent][1], par_reso[icent][2]);
  return ftemp->Eval(pt);
}
