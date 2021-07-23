#include <fstream>
#include <string>
#include <vector>

#include <TFile.h>
#include <TH2F.h>
#include <TCanvas.h>

#include "emix.h"
#include "forest.h"
#include "skimbranch.h"
#include "xjjrootuti.h"

int mixevents(std::string inputname_trig, std::string inputname_mb, 
              std::string outputname, int NMIX, int entries = -1)
{
  emix::mixhve mix(emix::N_HIBIN, emix::N_VZ, emix::N_EVENTPLANE);
  
  bool ishi = true;

  TFile* inf_mb = TFile::Open(inputname_mb.c_str());
  phoD::forest* f_mb = new phoD::forest(inf_mb, ishi);
  phoD::etree* etr_mb = f_mb->etr();
  TFile* inf_trig = TFile::Open(inputname_trig.c_str());           
  phoD::forest* f_trig = new phoD::forest(inf_trig, ishi, "akPu4PFJetAnalyzer/t");
  phoD::etree* etr_trig = f_trig->etr();
  phoD::jtree* jtr_trig = f_trig->jtr();
  TFile* outf = new TFile(outputname.c_str(), "recreate");

  phoD::skimbranch* sb = new phoD::skimbranch(ishi);
  std::map<std::string, phoD::skimtree*> s;
  s["hlt"] = new phoD::skimtree("hltanalysis/HltTree", inf_trig, outf, sb->branches("hltanalysis/HltTree"));
  s["skim"] = new phoD::skimtree("skimanalysis/HltTree", inf_trig, outf, sb->branches("skimanalysis/HltTree"));
  s["hi"] = new phoD::skimtree("hiEvtAnalyzer/HiTree", inf_trig, outf, sb->branches("hiEvtAnalyzer/HiTree"));
  s["forest"] = new phoD::skimtree("HiForest/HiForestInfo", inf_trig, outf, sb->branches("HiForest/HiForestInfo"));
  s["jet"] = new phoD::skimtree("akPu4PFJetAnalyzer/t", inf_trig, outf, sb->branches("akPu4PFJetAnalyzer/t"));
  s["ntDkpi"] = new phoD::skimtree("Dfinder/ntDkpi", inf_mb, outf, sb->branches("Dfinder/ntDkpi"));
  // outf->cd();

  xjjroot::mkdir(outputname);
  std::vector<int> outv[emix::N_HIBIN][emix::N_VZ][emix::N_EVENTPLANE];
  int current[emix::N_HIBIN][emix::N_VZ][emix::N_EVENTPLANE];
  for(int i=0; i<emix::N_HIBIN; i++) { for(int j=0; j<emix::N_VZ; j++) { for(int k=0; k<emix::N_EVENTPLANE; k++) { current[i][j][k] = 0; } } }
  TH2F* h_mb = new TH2F("h_mb", ";hiBin;vz", emix::N_HIBIN, 0, emix::N_HIBIN, emix::N_VZ, 0, emix::N_VZ);
  TH2F* h_trig = new TH2F("h_trig", ";hiBin;vz", emix::N_HIBIN, 0, emix::N_HIBIN, emix::N_VZ, 0, emix::N_VZ);
  TH2F* h_rep = new TH2F("h_rep", ";hiBin;vz", emix::N_HIBIN, 0, emix::N_HIBIN, emix::N_VZ, 0, emix::N_VZ);
  xjjroot::sethempty(h_mb, 0, -0.8);
  xjjroot::sethempty(h_trig, 0, -0.8);
  xjjroot::sethempty(h_rep, 0, -0.8);
  xjjroot::setgstyle(1);
  xjjroot::mkdir("plots/");
  gStyle->SetPadLeftMargin(xjjroot::margin_pad_left*0.3);
  TCanvas* c;

  // scan MB
  int nentries = etr_mb->GetEntries();
  for(int i=0; i<nentries; i++)
    {
      xjjc::progressslide(i, nentries, 10000);
      etr_mb->GetEntry(i);

      if(!etr_mb->evtsel()) continue;

      int ihibin = mix.ihibin(etr_mb->hiBin());
      if(ihibin < 0) { std::cout<<"error: bad hiBin."<<std::endl; return 2; }
      int ivz = mix.ivz(etr_mb->vz());
      if(ivz < 0) continue;
      int ieventplane = 0;
      outv[ihibin][ivz][ieventplane].push_back(i);
      h_mb->Fill(ihibin, ivz);
    }
  xjjc::progressbar_summary(nentries);

  c = new TCanvas("c", "", 1200, 600);
  h_mb->Draw("COL");
  c->SaveAs("plots/cemix_mb.pdf");
  delete c;

  // mix events
  int npass = 0;
  nentries = entries<0?etr_trig->GetEntries():std::min(entries, etr_trig->GetEntries());
  for(int i=0; i<nentries; i++)
    {
      xjjc::progressslide(i, nentries, 1000);
      for(auto& ss : s) { if(ss.first != "ntDkpi") { ss.second->GetEntry(i); } } // GetEntry -> trig

      if(!etr_trig->evtsel() || !etr_trig->hltsel_jet()) continue;

      // jet selection
      int jlead = -1;
      for(int j=0; j<jtr_trig->nref(); j++)
        {
          if((*jtr_trig)["jtpt"][j] <= 80 || fabs((*jtr_trig)["jteta"][j]) >= 1.6) continue;
          jlead = j;
          break;
        }
      if(jlead < 0) continue;

      int ihibin = mix.ihibin(etr_trig->hiBin());
      if(ihibin < 0) { std::cout<<"error: bad hiBin."<<std::endl; return 2; }
      int ivz = mix.ivz(etr_trig->vz());
      if(ivz < 0) continue;
      int ieventplane = 0;
      h_trig->Fill(ihibin, ivz);

      int nevt = outv[ihibin][ivz][ieventplane].size();
      if(nevt==0) { std::cout<<"error: bad mapping."<<std::endl; return 3; }

      npass++;

      for(int k=0; k<NMIX; k++)
        {
          if(current[ihibin][ivz][ieventplane] >= nevt) h_rep->Fill(ihibin, ivz);
          int now = current[ihibin][ivz][ieventplane]%nevt;
          int ievt = outv[ihibin][ivz][ieventplane][now];
          s["ntDkpi"]->GetEntry(ievt); // GetEntry -> mb

          for(auto& ss : s) { ss.second->Fill(); }

          current[ihibin][ivz][ieventplane]++;
        }
      h_trig->Fill(ihibin, ivz);
    }
  xjjc::progressbar_summary(nentries);

  TH2F* h_ratio = (TH2F*)h_mb->Clone("h_ratio");
  h_ratio->Divide(h_trig);
  xjjroot::sethempty(h_ratio, 0, -0.8);

  c = new TCanvas("c", "", 1200, 600);
  h_trig->Draw("COL");
  c->SaveAs("plots/cemix_trig.pdf");
  delete c;

  c = new TCanvas("c", "", 1200, 600);
  h_rep->Draw("COL TEXT");
  c->SaveAs("plots/cemix_rep.pdf");
  delete c;

  c = new TCanvas("c", "", 1200, 600);
  h_ratio->Draw("COL TEXT");
  c->SaveAs("plots/cemix_ratio.pdf");
  delete c;

  outf->cd();
  outf->Write();
  outf->Close();

  std::cout<<npass<<" x "<<NMIX<<" = "<<npass*NMIX<<std::endl;

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc==6) return mixevents(argv[1], argv[2], argv[3], atoi(argv[4]), atoi(argv[5]));
  if(argc==5) return mixevents(argv[1], argv[2], argv[3], atoi(argv[4]));
  return 1;
}
