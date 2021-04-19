#include <TFile.h>
#include <TH1F.h>
#include <TCanvas.h>
#include <TMath.h>
#include <string>
#include <vector>

#include "jtreso.h"
#include "jetreso.h"
#include "para.h"
#include "param.h"

#include "bins.h"
#include "xjjcuti.h"
#include "xjjanauti.h"

namespace jtreso
{
  void fitreso(TH1F* h, xjjroot::param& pa, Color_t cc, TLegend* l);
}

int jtreso_drawhist(std::string inputname, std::string outsubdir)
{
  TFile* inf = TFile::Open(inputname.c_str());
  Djet::param Djetpa(inf);
  xjjroot::param pa(Djetpa.ishi(), 0, 0, 0, 0, 0, 0, 1, "gen jet");

  xjjana::bins<float> veta(jtreso::jtetas),
    vcent(pa.ishi()?jtreso::cents_PbPb:jtreso::cents_pp);

  auto hreso_phi = xjjc::array2d<TH1F*>(vcent.n(), veta.n());
  auto hreso_eta = xjjc::array2d<TH1F*>(vcent.n(), veta.n());
  std::map<std::string, TLegend*> leg; 
  for(int c=0; c<vcent.n(); c++)
    {
      pa.setcent(vcent[c], vcent[c+1]);

      for(int e=0; e<veta.n(); e++)
        {
          hreso_phi[c][e] = xjjroot::gethist<TH1F>(inf, Form("hreso_phi%s_%d", (pa.ishi()?Form("_%d", c):""), e));
          hreso_eta[c][e] = xjjroot::gethist<TH1F>(inf, Form("hreso_eta%s_%d", (pa.ishi()?Form("_%d", c):""), e));
          xjjroot::sethempty(hreso_phi[c][e], 0, 0.5);
          xjjroot::sethempty(hreso_eta[c][e], 0, 0.5);
          // hreso_phi[c][e]->GetXaxis()->SetNdivisions(-505);
          // hreso_eta[c][e]->GetXaxis()->SetNdivisions(-505);
        }       
      xjjana::sethsminmax(hreso_phi[c], 0.8, 1.2);
      xjjana::sethsminmax(hreso_eta[c], 0.8, 1.2);

      xjjroot::setgstyle(2);

      TCanvas* c_phi = new TCanvas("cfreso_phi", "", 600, 600);
      leg[c_phi->GetName()] = new TLegend(0.60, 0.87-0.039*veta.n(), 0.90, 0.87);
      xjjroot::setleg(leg[c_phi->GetName()], 0.032);
      hreso_phi[c][0]->Draw("AXIS");
      xjjroot::drawtex(0.22, 0.87, pa.tag("cent").c_str(), 0.038, 13);

      TCanvas* c_eta = new TCanvas("cfreso_eta", "", 600, 600); 
      leg[c_eta->GetName()] = new TLegend(0.60, 0.87-0.039*veta.n(), 0.90, 0.87);
      xjjroot::setleg(leg[c_eta->GetName()], 0.032);
      hreso_eta[c][0]->Draw("AXIS");
      xjjroot::drawtex(0.22, 0.87, pa.tag("cent").c_str(), 0.038, 13);

      for(int e=0; e<veta.n(); e++)
        {
          pa.seteta(veta[e], veta[e+1]);
          c_phi->cd();
          jtreso::fitreso(hreso_phi[c][e], pa, veta.n()==1?kBlack:xjjroot::colorlist_middle[e], leg[c_phi->GetName()]);
          c_eta->cd();
          jtreso::fitreso(hreso_eta[c][e], pa, veta.n()==1?kBlack:xjjroot::colorlist_middle[e], leg[c_eta->GetName()]);
        }
      for(auto& ca : {c_phi, c_eta})
        {
          ca->cd();
          xjjroot::drawCMSleft("Simulation");
          xjjroot::drawCMSright(pa.tag("ishi"));
          leg[ca->GetName()]->Draw();
          std::string output = Form("plots/%s_%s/%s%s.pdf", outsubdir.c_str(), pa.tag("ishi").c_str(), ca->GetName(), (pa.ishi()?Form("_%d", c):""));
          xjjroot::mkdir(output);
          ca->SaveAs(output.c_str());
          delete ca;
        }
    }
  return 0;
}

void jtreso::fitreso(TH1F* h, xjjroot::param& pa, Color_t cc, TLegend* l)
{
  xjjroot::setthgrstyle(h, cc, 21, 0.8, cc, 1, 2);

  Djet::jetreso* r = new Djet::jetreso(pa.ishi());
  TF1* freso = r->freso();

  freso->SetParameters(0.01, 0.05, 1.);
  freso->SetParLimits(0, 0, 0.5);
  freso->SetParLimits(1, 0, 0.5);
  freso->SetParLimits(2, 0, 10.);
  if(!pa.ishi())
    freso->FixParameter(2, 0);

  xjjroot::setlinestyle(freso, cc, 1, 2);
  h->Draw("pe X0 same");
  h->Fit("freso", "", "", 80, h->GetXaxis()->GetXmax());

  l->AddEntry(h, pa.tag("eta").c_str(), "pl");

  delete r;
}

int main(int argc, char* argv[])
{
  if(argc==3)
    return jtreso_drawhist(argv[1], argv[2]);
  return 1;
}
