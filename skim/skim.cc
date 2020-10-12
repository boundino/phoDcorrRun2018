#include <TFile.h>
#include <TTree.h>

#include <string>
#include <map>

#include "skimbranch.h"
#include "dtree.h"
#include "dmva.h"
#include "xjjcuti.h"

int skim(std::string inputname, std::string outputname,
         int ishi, int evtfilt, int mvafilt, int hltfilt)
{
  TFile* inf = TFile::Open(inputname.c_str());
  TFile* outf = new TFile(outputname.c_str(), "recreate");

  phoD::skimbranch* sb = new phoD::skimbranch(ishi);
  std::map<std::string, phoD::skimtree*> s;
  s["hlt"] = new phoD::skimtree("hltanalysis/HltTree", inf, outf, sb->branches("hltanalysis/HltTree"));
  if(ishi) s["obj"] = new phoD::skimtree("hltobject/HLT_HIGEDPhoton40_v", inf, outf, sb->branches("hltobject/HLT_HIGEDPhoton40_v"));
  // else s["obj"] = new phoD::skimtree("hltobject/HLT_HIPhoton40_HoverELoose_v", inf, outf, sb->branches("hltobject/HLT_HIPhoton40_HoverELoose_v"));
  s["ggGED"] = new phoD::skimtree("ggHiNtuplizerGED/EventTree", inf, outf, sb->branches("ggHiNtuplizerGED/EventTree"));
  s["skim"] = new phoD::skimtree("skimanalysis/HltTree", inf, outf, sb->branches("skimanalysis/HltTree"));
  s["hi"] = new phoD::skimtree("hiEvtAnalyzer/HiTree", inf, outf, sb->branches("hiEvtAnalyzer/HiTree"));
  s["forest"] = new phoD::skimtree("HiForest/HiForestInfo", inf, outf, sb->branches("HiForest/HiForestInfo"));
  s["ntGen"] = new phoD::skimtree("Dfinder/ntGen", inf, outf, sb->branches("Dfinder/ntGen"));
  TTree* nt = (TTree*)inf->Get("Dfinder/ntDkpi");
  phoD::dtree* dt = new phoD::dtree(nt, ishi);
  phoD::dtree* dt_new = new phoD::dtree(outf, "Dfinder/ntDkpi", ishi);

  // hiBin
  int hiBin = -1;
  if(ishi) { s["hi"]->t()->SetBranchAddress("hiBin", &hiBin); }
  // event filter
  std::map<std::string, int> vevt;
  if(ishi)
    {
      s["skim"]->t()->SetBranchAddress("pprimaryVertexFilter", &(vevt["pprimaryVertexFilter"]));
      s["skim"]->t()->SetBranchAddress("phfCoincFilter2Th4", &(vevt["phfCoincFilter2Th4"]));
      s["skim"]->t()->SetBranchAddress("pclusterCompatibilityFilter", &(vevt["pclusterCompatibilityFilter"]));
    }
  else
    {
      s["skim"]->t()->SetBranchAddress("pBeamScrapingFilter", &(vevt["pBeamScrapingFilter"]));
      s["skim"]->t()->SetBranchAddress("pPAprimaryVertexFilter", &(vevt["pPAprimaryVertexFilter"]));
    }
  // hlt filter
  std::map<std::string, int> vhlt;
  if(ishi)
    {
      s["hlt"]->t()->SetBranchAddress("HLT_HIGEDPhoton40_v1", &(vhlt["HLT_HIGEDPhoton40_v1"]));
    }
  else
    {
      s["hlt"]->t()->SetBranchAddress("HLT_HIPhoton40_HoverELoose_v1", &(vhlt["HLT_HIPhoton40_HoverELoose_v1"]));
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

      // event selection [and]
      if(evtfilt)
        {
          int sflag = 1;
          for(auto& e : vevt) { if(!e.second) { sflag = 0; break; } }
          if(!sflag) continue;
        }
      // hlt selection [or]
      if(hltfilt)
        {
          int sflag = 0;
          for(auto& h : vhlt) { if(h.second) { sflag++; break; } }
          if(!sflag) continue;
        }

      for(auto& ss : s) { if(ss.first != "forest") { ss.second->Fill(); } }

      dt_new->ClearDsize();
      for(int j=0; j<dt->Dsize(); j++)
        {
          if(!dt->presel(j)) continue;

          switch(mvafilt)
            {
            case 1:
              {
                float mvaval = mva->eval(dt, j, hiBin);
                if(!mva->pass(mvaval, dt->val<float>("Dpt", j), hiBin)) continue;
                dt_new->Fillone("BDT", mvaval);
              }
            case 2:
              {
                if(!dt->val<float>("DlxyBS", j)/dt->val<float>("DlxyBSErr", j) <= 3.5 ||
                   dt->val<float>("DdthetaBScorr", j) >= 0.2
                   ) continue;
              }
            case 3:
              {
                if(dt->val<int>("Dgen", j) != 23333 && dt->val<int>("Dgen", j) != 23344) continue;
              }
            }
          dt_new->Fillall(dt, j);
          dt_new->Dsizepp();
        }
      dt_new->Fill();
    }
  xjjc::progressbar_summary(nentries);

  outf->Write();
  outf->Close();

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc==7) { return skim(argv[1], argv[2],
                            atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), atoi(argv[6])); }
  return 1;
}
