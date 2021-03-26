#include <TFile.h>
#include <TTree.h>

#include <string>
#include <map>

#include "skimbranch.h"
#include "dtree.h"
#include "jtree.h"
#include "dmva.h"
#include "xjjcuti.h"

int skim(std::string inputname, std::string outputname,
         int ishi, int evtfilt, int hltfilt, int mvafilt,
         float jetptcut, bool removeevent)
{
  TFile* inf = TFile::Open(inputname.c_str());
  TFile* outf = new TFile(outputname.c_str(), "recreate");

  phoD::skimbranch* sb = new phoD::skimbranch(ishi);
  std::map<std::string, phoD::skimtree*> s;
  s["hlt"] = new phoD::skimtree("hltanalysis/HltTree", inf, outf, sb->branches("hltanalysis/HltTree"));
  s["skim"] = new phoD::skimtree("skimanalysis/HltTree", inf, outf, sb->branches("skimanalysis/HltTree"));
  s["hi"] = new phoD::skimtree("hiEvtAnalyzer/HiTree", inf, outf, sb->branches("hiEvtAnalyzer/HiTree"));
  s["forest"] = new phoD::skimtree("HiForest/HiForestInfo", inf, outf, sb->branches("HiForest/HiForestInfo"));
  s["ntGen"] = new phoD::skimtree("Dfinder/ntGen", inf, outf, sb->branches("Dfinder/ntGen"));
  phoD::dtree* dt = new phoD::dtree((TTree*)inf->Get("Dfinder/ntDkpi"), ishi);
  phoD::dtree* dt_new = new phoD::dtree(outf, "Dfinder/ntDkpi", ishi);
  std::map<std::string, phoD::jtree*> jt, jt_new;
  if(ishi)
    {
      jt["4Calo"] = new phoD::jtree((TTree*)inf->Get("akPu4CaloJetAnalyzer/t"), ishi);
      jt_new["4Calo"] = new phoD::jtree(outf, "akPu4CaloJetAnalyzer/t", ishi, jt["4Calo"]->isMC(), jt["4Calo"]->nt());
      jt["4PF"] = new phoD::jtree((TTree*)inf->Get("akPu4PFJetAnalyzer/t"), ishi);
      jt_new["4PF"] = new phoD::jtree(outf, "akPu4PFJetAnalyzer/t", ishi, jt["4PF"]->isMC(), jt["4PF"]->nt());
      jt["3PF"] = new phoD::jtree((TTree*)inf->Get("akPu3PFJetAnalyzer/t"), ishi);
      jt_new["3PF"] = new phoD::jtree(outf, "akPu3PFJetAnalyzer/t", ishi, jt["3PF"]->isMC(), jt["3PF"]->nt());
    }
  else
    {
      jt["4Calo"] = new phoD::jtree((TTree*)inf->Get("ak4CaloJetAnalyzer/t"), ishi);
      jt_new["4Calo"] = new phoD::jtree(outf, "ak4CaloJetAnalyzer/t", ishi, jt["4Calo"]->isMC(), jt["4Calo"]->nt());
      jt["4PF"] = new phoD::jtree((TTree*)inf->Get("ak4PFJetAnalyzer/t"), ishi);
      jt_new["4PF"] = new phoD::jtree(outf, "ak4PFJetAnalyzer/t", ishi, jt["4PF"]->isMC(), jt["4PF"]->nt());
      jt["3PF"] = new phoD::jtree((TTree*)inf->Get("ak3PFJetAnalyzer/t"), ishi);
      jt_new["3PF"] = new phoD::jtree(outf, "ak3PFJetAnalyzer/t", ishi, jt["3PF"]->isMC(), jt["3PF"]->nt());
    }
  // hiBin
  int hiBin = -1;
  if(ishi) { s["hi"]->t()->SetBranchAddress("hiBin", &hiBin); }
  // event filter
  std::map<std::string, std::map<bool, std::vector<std::string>>> path;
  path["skim"][true] = std::vector<std::string>{"pprimaryVertexFilter", "phfCoincFilter2Th4", "pclusterCompatibilityFilter"}; // HI
  path["skim"][false] = std::vector<std::string>{"pBeamScrapingFilter", "pPAprimaryVertexFilter"}; // pp
  path["hlt"][true] = std::vector<std::string>{"HLT_HIPuAK4CaloJet80Eta5p1_v1", "HLT_HIPuAK4CaloJet100Eta5p1_v1"}; // HI
  path["hlt"][false] = std::vector<std::string>{"HLT_HIAK4CaloJet80_v1"}; // pp

  std::map<std::string, std::map<std::string, int>> vevt;
  for(auto& sh : path)
    {
      auto sorh = sh.first;
      auto vv = sh.second[ishi];
      for(auto& p : vv)
        s[sorh]->t()->SetBranchAddress(p.c_str(), &(vevt[sorh][p]));
    }
  s["forest"]->GetEntry(0);
  s["forest"]->Fill();

  // MVA
  phoD::dmva* mva = new phoD::dmva("mva/");

  int nentries = s["hi"]->nentries();
  for(int i=0; i<nentries; i++)
    {
      if(i%100==0 || i==nentries-1) xjjc::progressbar(i, nentries);
      for(auto& ss : s) { if(ss.first != "forest") { ss.second->GetEntry(i); } }
      dt->GetEntry(i);
      for(auto& jj : jt) jj.second->GetEntry(i);

      // event selection [and]
      if(evtfilt)
        {
          int sflag = 1;
          for(auto& e : vevt["skim"]) { if(!e.second) { sflag = 0; break; } }
          if(!sflag) continue;
        }
      // hlt selection [or]
      if(hltfilt)
        {
          int sflag = 0;
          for(auto& e : vevt["hlt"]) { if(e.second) { sflag++; break; } }
          if(!sflag) continue;
        }

      // jet
      for(auto& jj : jt_new) jj.second->Clearnrefngen();
      // fill jet
      for(auto& jj : jt)
        {
          std::string t = jj.first;
          for(int j=0; j<jt[t]->nref(); j++)
            { 
              bool fill = jt[t]->val<float>("jtpt", j) > jetptcut;
              if(!fill) continue;
              jt_new[t]->Fillall_nref(jt[t], j);
              jt_new[t]->nrefpp();
            }
        }
      int njet = 0;
      for(auto& jj : jt_new) njet += jj.second->nref(); 
      if(removeevent && njet==0) continue;
      // fill gen jet
      for(auto& jj : jt)
        {
          std::string t = jj.first;
          if(!jt[t]->isMC()) break;
          for(int j=0; j<jt[t]->ngen(); j++)
            { 
              bool fill = jt[t]->val<float>("genpt", j) > jetptcut;
              if(!fill) continue;
              jt_new[t]->Fillall_ngen(jt[t], j);
              jt_new[t]->ngenpp();
            }
        }

      // fill D
      dt_new->ClearDsize();
      for(int j=0; j<dt->Dsize(); j++)
        {
          if(!dt->presel(j)) continue;

          bool fill = true;
          switch(mvafilt)
            {
            case 1:
              {
                float mvaval = mva->eval(dt, j, hiBin);
                if(mva->pass(mvaval, dt->val<float>("Dpt", j), hiBin))
                  {
                    dt_new->Fillone("BDT", mvaval);
                  }
                else
                  fill = false;
                break;
              }
            case 2:
              {
                if(dt->val<float>("DlxyBS", j)/dt->val<float>("DlxyBSErr", j) <= 3.5 ||
                   dt->val<float>("DdthetaBScorr", j) >= 0.2
                   ) fill = false;
                break;
              }
            case 3:
              {
                if(dt->val<int>("Dgen", j) != 23333 && dt->val<int>("Dgen", j) != 23344)
                  fill = false;
                break;
              }
            }
          if(!fill) continue;
          // std::cout<<"  --> filled"<<std::endl;
          dt_new->Fillall(dt, j);
          dt_new->Dsizepp();
        }

      if(removeevent && dt_new->Dsize()==0) continue;

      // ---------------------------- Fill ----------------------------
      for(auto& ss : s) { if(ss.first != "forest") { ss.second->Fill(); } }
      for(auto& jj : jt_new) { jt_new[jj.first]->Fill(); }
      dt_new->Fill();

    }
  xjjc::progressbar_summary(nentries);

  outf->Write();
  outf->Close();

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc==9) { return skim(argv[1], argv[2],
                            atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), atoi(argv[6]),
                            atof(argv[7]), atoi(argv[8])); }
  return 1;
}
