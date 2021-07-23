#include <TFile.h>
#include <TCanvas.h>
#include <TH2F.h>
#include <iostream>
#include <iomanip>

#include "para.h"

#include "xjjrootuti.h"

int comp_drawhist()
{
  std::vector<float> ptmin={2, 3, 4, 5, 6, 10, 15, 20, 25, 30};
  std::vector<float> ptmax={10, 15, 20, 25, 30, 50, 100};

  std::map<std::string, std::string> inputname;
  std::map<std::string, TFile*> inf;
  std::map<std::string, std::map<std::string, TH1F*>> hd;
  std::map<std::string, TH2F*> hsig, hsigaa;
  for(auto& v : Djet::var)
    {
    hsig[v] = new TH2F(Form("hsig_%s", v.c_str()), ";D p_{T}^{min} (GeV/c);D p_{T}^{max} (GeV/c)", ptmin.size(), 0, ptmin.size(), ptmax.size(), 0, ptmax.size());
    hsigaa[v] = new TH2F(Form("hsigaa_%s", v.c_str()), ";D p_{T}^{min} (GeV/c);D p_{T}^{max} (GeV/c)", ptmin.size(), 0, ptmin.size(), ptmax.size(), 0, ptmax.size());
    }
  for(int i=0; i<ptmin.size(); i++)
    {
      float pti = ptmin[i];
      for(auto& v : Djet::var)
        hsig[v]->GetXaxis()->SetBinLabel(i+1, Form("%.0f", pti));
      for(int j=0; j<ptmax.size(); j++)
        {
          float ptj = ptmax[j];
          if(!i)
            {
              for(auto& v : Djet::var)
                hsig[v]->GetYaxis()->SetBinLabel(j+1, Form("%.0f", ptj));
            }
          if(ptj <= pti) continue;

          Djet::param pa(0, pti, ptj, 1.2, 0, 90, 80, 1.6, 0);
          inputname["pp"] = "rootfiles/ana_finebin_trig_" + pa.tag() + "/calchist.root";
          std::cout<<inputname["pp"]<<std::endl;
          pa.setishi(1);
          inputname["PbPb"] = "rootfiles/ana_finebin_trig_" + pa.tag() + "/calchist.root";
          std::cout<<inputname["PbPb"]<<std::endl;

          for(auto& v : Djet::var)
            {
              for(auto& t : {"pp", "PbPb"})
                {
                  inf[t] = TFile::Open(inputname[t].c_str());
                  hd[t][v] = xjjroot::gethist<TH1F>(inf[t], "h"+v+"_sbr_effcorr");
                  hd[t][v]->SetName(Form("h%s_%s", t, v.c_str()));
                }
              hsig[v]->Fill(i, j, hd["pp"][v]->GetBinContent(1) / hd["PbPb"][v]->GetBinError(1));
              hsigaa[v]->Fill(i, j, hd["PbPb"][v]->GetBinContent(1) / hd["PbPb"][v]->GetBinError(1));
            }
        }
    }

  for(auto& v : Djet::var)
    {
      xjjroot::setgstyle(1);
      TCanvas* c = new TCanvas("c", "", 800, 600);
      xjjroot::sethempty(hsig[v]);
      hsig[v]->Draw("col TEXT");
      std::string output = "plots/sigscan/c"+v+".pdf";
      xjjroot::mkdir(output);
      c->SaveAs(output.c_str());
      delete c;
    }

  for(auto& v : Djet::var)
    {
      xjjroot::setgstyle(1);
      TCanvas* c = new TCanvas("c", "", 800, 600);
      xjjroot::sethempty(hsig[v]);
      hsigaa[v]->Draw("col TEXT");
      std::string output = "plots/sigscan/c"+v+"_aa.pdf";
      xjjroot::mkdir(output);
      c->SaveAs(output.c_str());
      delete c;
    }

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc==1)
    return comp_drawhist();
  return 1;
}
