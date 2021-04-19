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
  TF1* fitreso(TH1F* h, std::string output, xjjroot::param& pa);
}

int jtreso_fithist(std::string inputname, std::string outsubdir)
{
  TFile* inf = TFile::Open(inputname.c_str());
  Djet::param pa(inf);

  xjjana::bins<float> vpt(jtreso::jtpts), veta(jtreso::jtetas),
    vcent(pa.ishi()?jtreso::cents_PbPb:jtreso::cents_pp);

  auto hphi = xjjc::array3d<TH1F*>(vcent.n(), veta.n(), vpt.n());
  auto heta = xjjc::array3d<TH1F*>(vcent.n(), veta.n(), vpt.n());
  auto hreso_phi = xjjc::array2d<TH1F*>(vcent.n(), veta.n());
  auto hreso_eta = xjjc::array2d<TH1F*>(vcent.n(), veta.n());
  for(int c=0; c<vcent.n(); c++)
    for(int e=0; e<veta.n(); e++)
      {
        hreso_phi[c][e] = new TH1F(Form("hreso_phi%s_%d", (pa.ishi()?Form("_%d", c):""), e), ";Gen jet p_{T} (GeV/c);#sigma(#phi^{reco} - #phi^{gen})", vpt.n(), vpt.v().data());
        hreso_eta[c][e] = new TH1F(Form("hreso_eta%s_%d", (pa.ishi()?Form("_%d", c):""), e), ";Gen jet p_{T} (GeV/c);#sigma(#eta^{reco} - #eta^{gen})", vpt.n(), vpt.v().data());
        for(int p=0; p<vpt.n(); p++)
          {
            hphi[c][e][p] = xjjroot::gethist<TH1F>(inf, Form("hphi%s_%d_%d", (pa.ishi()?Form("_%d", c):""), e, p));
            heta[c][e][p] = xjjroot::gethist<TH1F>(inf, Form("heta%s_%d_%d", (pa.ishi()?Form("_%d", c):""), e, p));

            xjjroot::param para(pa.ishi(), vpt[p], vpt[p+1], veta[e], veta[e+1], vcent[c], vcent[c+1], 1, "gen jet");
            TF1* f;
            f = jtreso::fitreso(hphi[c][e][p], 
                                Form("plots/%s_%s/idx_phi/creso%s_%d_%d.pdf", outsubdir.c_str(), pa.tag("ishi").c_str(), (pa.ishi()?Form("_%d", c):""), e, p),
                                para);
            hreso_phi[c][e]->SetBinContent(p+1, f->GetParameter(2));
            hreso_phi[c][e]->SetBinError(p+1, f->GetParError(2));
            delete f;

            f = jtreso::fitreso(heta[c][e][p], 
                                Form("plots/%s_%s/idx_eta/creso%s_%d_%d.pdf", outsubdir.c_str(), pa.tag("ishi").c_str(), (pa.ishi()?Form("_%d", c):""), e, p),
                                para);
            hreso_eta[c][e]->SetBinContent(p+1, f->GetParameter(2));
            hreso_eta[c][e]->SetBinError(p+1, f->GetParError(2));
            delete f;
          }
      }

  std::string outputname = "rootfiles/" + outsubdir + "_" + pa.tag("ishi") + "/fithist.root";
  xjjroot::mkdir(outputname);
  TFile* outf = new TFile(outputname.c_str(), "recreate");
  for(auto& h : hreso_phi) for(auto& hh : h) xjjroot::writehist(hh, 15);
  for(auto& h : hreso_eta) for(auto& hh : h) xjjroot::writehist(hh, 15);
  pa.write();
  outf->Close();

  return 0;
}

TF1* jtreso::fitreso(TH1F* h, std::string output, xjjroot::param& pa)
{
  h->Scale(1./h->Integral(), "width");
  xjjroot::setthgrstyle(h, kBlack, 21, 0.8, kBlack, 1, 2);
  TH1F* hclone = xjjana::copyobject(h, "hclone");
  xjjroot::sethempty(h, 0, 0);
  h->SetMinimum(0);
  h->SetMaximum(xjjana::gethmaximum(h)*1.4);
  h->GetXaxis()->SetNdivisions(-404);

  Djet::jetreso* r = new Djet::jetreso(pa.ishi());
  TF1* f = r->f();
  f->SetParameters(1, 0, h->GetXaxis()->GetXmax()/2., 0.5, 0.5);
  f->SetParLimits(0, 0, 2);
  f->SetParLimits(1, h->GetXaxis()->GetXmin(), h->GetXaxis()->GetXmax());
  f->SetParLimits(2, h->GetXaxis()->GetXmax()/100., h->GetXaxis()->GetXmax());
  f->SetParLimits(3, 0, 1);
  f->SetParLimits(4, 0, 1);

  // float middlept = (pa["ptmin"]+pa["ptmax"])/2.;
  // f->FixParameter(3, r->par3());
  // f->FixParameter(4, r->par4(middlept));
  if(pa.ishi())
    {
      f->FixParameter(3, 1);
      f->FixParameter(4, 1);
    }
  else
    {
      f->FixParameter(3, r->par3());
      f->FixParameter(4, r->par4());
    }
  xjjroot::setlinestyle(f, xjjroot::mycolor_satmiddle2["red"], 1, 2);
  f->SetNpx(1000);
  xjjroot::setgstyle(2);
  TCanvas* c = new TCanvas("c", "", 600, 600);
  h->Draw("pe");
  h->Fit("f", "q", "", h->GetXaxis()->GetXmin(), h->GetXaxis()->GetXmax());
  r->drawfcomp(h->GetXaxis()->GetXmin(), h->GetXaxis()->GetXmax());
  hclone->Draw("pe same");
  pa.drawtex();
  xjjroot::drawtex(0.63, 0.87, Form("#mu = %.4f", f->GetParameter(1)), 0.038, 13);
  xjjroot::drawtex(0.63, 0.87-0.038*1.4, Form("#sigma = %.4f", f->GetParameter(2)), 0.038, 13);
  xjjroot::drawCMSleft("Simulation");
  xjjroot::drawCMSright(pa.tag("ishi"));
  xjjroot::mkdir(output);
  c->SaveAs(output.c_str());

  TF1* fclone = xjjana::copyobject(f, "fclone");

  delete c;
  delete hclone;
  delete r;
  return fclone;
}

int main(int argc, char* argv[])
{
  if(argc==3)
    return jtreso_fithist(argv[1], argv[2]);
  return 1;
}
