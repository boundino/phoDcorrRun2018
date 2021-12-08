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
              std::string outputname, int NMIX, float fraction = 1., float ifrac = 0.)
{
  emix::mixhve mix(emix::N_HIBIN, emix::N_VZ, emix::N_EVENTPLANE);
  
  bool ishi = true;

  TFile* inf_mb = TFile::Open(inputname_mb.c_str());
  phoD::forest* f_mb = new phoD::forest(inf_mb, ishi, "akCs4PFJetAnalyzer/t");
  phoD::etree* etr_mb = f_mb->etr();
  phoD::jtree* jtr_mb = f_mb->jtr();
  TFile* inf_trig = TFile::Open(inputname_trig.c_str());           
  // phoD::forest* f_trig = new phoD::forest(inf_trig, ishi, "akPu4PFJetAnalyzer/t");
  phoD::forest* f_trig = new phoD::forest(inf_trig, ishi, "akCs4PFJetAnalyzer/t");
  phoD::etree* etr_trig = f_trig->etr();
  phoD::jtree* jtr_trig = f_trig->jtr();
  TH1F* hnevt = (TH1F*)inf_trig->Get("hnentries");
  int nentries_raw = hnevt->GetEntries(); delete hnevt;

  TFile* outf = new TFile(outputname.c_str(), "recreate");

  phoD::skimbranch* sb = new phoD::skimbranch(ishi);
  std::map<std::string, phoD::skimtree*> s;
  s["hlt"] = new phoD::skimtree("hltanalysis/HltTree", inf_trig, outf, sb->branches("hltanalysis/HltTree"));
  s["skim"] = new phoD::skimtree("skimanalysis/HltTree", inf_trig, outf, sb->branches("skimanalysis/HltTree"));
  s["hi"] = new phoD::skimtree("hiEvtAnalyzer/HiTree", inf_trig, outf, sb->branches("hiEvtAnalyzer/HiTree"));
  // s["forest"] = new phoD::skimtree("HiForest/HiForestInfo", inf_trig, outf, sb->branches("HiForest/HiForestInfo"));
  // s["forest"] = new phoD::skimtree("HiForestInfo/HiForest", inf_trig, outf, sb->branches("HiForestInfo/HiForest"));
  // s["jet"] = new phoD::skimtree("akPu4PFJetAnalyzer/t", inf_trig, outf, sb->branches("akPu4PFJetAnalyzer/t"));
  s["jet"] = new phoD::skimtree("akCs4PFJetAnalyzer/t", inf_trig, outf, sb->branches("akCs4PFJetAnalyzer/t"));
  s["ntDkpi"] = new phoD::skimtree("Dfinder/ntDkpi", inf_mb, outf, sb->branches("Dfinder/ntDkpi"));
  // outf->cd();

  xjjroot::mkdir(outputname);
  std::vector<int> outv[emix::N_HIBIN][emix::N_VZ][emix::N_EVENTPLANE];
  int current[emix::N_HIBIN][emix::N_VZ][emix::N_EVENTPLANE];
  for(int i=0; i<emix::N_HIBIN; i++) { for(int j=0; j<emix::N_VZ; j++) { for(int k=0; k<emix::N_EVENTPLANE; k++) { current[i][j][k] = 0; } } }
  TH2F* h_mb = new TH2F("h_mb", ";hiBin;vz", emix::N_HIBIN, 0, emix::N_HIBIN, emix::N_VZ, 0, emix::N_VZ);
  TH2F* h_trig = new TH2F("h_trig", ";hiBin;vz", emix::N_HIBIN, 0, emix::N_HIBIN, emix::N_VZ, 0, emix::N_VZ);
  TH2F* h_rep = new TH2F("h_rep", ";hiBin;vz", emix::N_HIBIN, 0, emix::N_HIBIN, emix::N_VZ, 0, emix::N_VZ);

  // scan MB
  int npass_mb = 0;
  int nentries_mb_frac = std::floor(etr_mb->GetEntries()/fraction);
  for(int i=ifrac*nentries_mb_frac; i<(ifrac+1)*nentries_mb_frac; i++)
    {
      // std::cout<<i<<std::endl;
      xjjc::progressslide(i-ifrac*nentries_mb_frac, nentries_mb_frac, 100000);
      etr_mb->GetEntry(i);
      // jtr_mb->GetEntry(i);

      if(!etr_mb->evtsel()) continue;
      // if(jtr_mb->nref()>0) continue; // only use MB events without 60 GeV jets

      int ihibin = mix.ihibin(etr_mb->hiBin());
      if(ihibin < 0) { std::cout<<"error: bad hiBin."<<std::endl; return 2; }
      int ivz = mix.ivz(etr_mb->vz());
      if(ivz < 0) continue;
      int ieventplane = mix.ieventplane(etr_mb->hiEvtPlane());
      // std::cout<<etr_mb->hiEvtPlane()<<" -> "<<ieventplane<<std::endl;
      if(ieventplane < 0) { std::cout<<"error: bad eventplane."<<std::endl; return 2; }
      outv[ihibin][ivz][ieventplane].push_back(i);
      h_mb->Fill(ihibin, ivz);
      npass_mb++;
    }
  xjjc::progressbar_summary(nentries_mb_frac);
  std::cout<<"npass_mb = "<<npass_mb<<std::endl;

  for(int i=0; i<emix::N_HIBIN; i++)
    for(int j=0; j<emix::N_VZ; j++)
      for(int k=0; k<emix::N_EVENTPLANE; k++)
        if(outv[i][j][k].size() == 0)
          std::cout<<"emix::mixhve: mapping empty bin ("<<i<<", "<<j<<", "<<k<<")"<<std::endl;

  // mix events
  int npass = 0;
  int nentries_trig_frac = std::floor(etr_trig->GetEntries()/fraction);
  int nentries_equiv = (int)(nentries_raw * 1. nentries_trig_frac/etr_trig->GetEntries());
  for(int i=0; i<nentries_equiv; i++) hnentries->Fill(0.5);

  for(int i=ifrac*nentries_trig_frac; i<(ifrac+1)*nentries_trig_frac; i++)
    {
      xjjc::progressslide(i-ifrac*nentries_trig_frac, nentries_trig_frac, 10000);
      for(auto& ss : s) { if(ss.first != "ntDkpi") { ss.second->GetEntry(i); } } // GetEntry -> trig

      if(!etr_trig->evtsel()) continue;

      // jet selection
      int jlead = -1;
      for(int j=0; j<jtr_trig->nref(); j++)
        {
          // if((*jtr_trig)["jtpt"][j] <= 80 || fabs((*jtr_trig)["jteta"][j]) >= 1.6) continue;
          if(fabs((*jtr_trig)["jteta"][j]) >= 2.) continue;
          jlead = j;
          break;
        }
      if(jlead < 0) continue;

      int ihibin = mix.ihibin(etr_trig->hiBin());
      if(ihibin < 0) { std::cout<<"error: bad hiBin."<<std::endl; return 2; }
      int ivz = mix.ivz(etr_trig->vz());
      if(ivz < 0) { std::cout<<"error: bad vz."<<std::endl; continue; }
      int ieventplane = mix.ieventplane(etr_trig->hiEvtPlane());
      if(ieventplane < 0) { std::cout<<"error: bad eventplane."<<std::endl; continue; }
      h_trig->Fill(ihibin, ivz);

      int nevt = outv[ihibin][ivz][ieventplane].size();
      if(nevt==0) { std::cout<<"error: bad mapping: ("<<ihibin<<", "<<ivz<<", "<<ieventplane<<")"<<std::endl; continue; }

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
  xjjc::progressbar_summary(nentries_trig_frac);

  TH2F* h_ratio = (TH2F*)h_mb->Clone("h_ratio");
  h_ratio->Divide(h_trig);

  outf->cd();
  outf->Write();
  outf->Close();

  std::cout<<npass<<" x "<<NMIX<<" = "<<npass*NMIX<<std::endl;

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc==7) 
    return mixevents(argv[1], argv[2], argv[3], atoi(argv[4]), atof(argv[5]), atof(argv[6]));
  return 1;
}
