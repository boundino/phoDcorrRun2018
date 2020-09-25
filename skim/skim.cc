#include <TFile.h>
#include <TTree.h>

#include <string>
#include <map>

#include "skimbranch.h"
#include "dtree.h"
#include "dmva.h"
#include "xjjcuti.h"

int skim(std::string inputname, std::string outputname)
{
  TFile* inf = TFile::Open(inputname.c_str());
  TFile* outf = new TFile(outputname.c_str(), "recreate");

  phoD::skimbranch* sb = new phoD::skimbranch();
  std::map<std::string, phoD::skimtree*> s;
  s["hlt"] = new phoD::skimtree("hltanalysis/HltTree", inf, outf, sb->branches("hltanalysis/HltTree"));
  s["obj"] = new phoD::skimtree("hltobject/HLT_HIGEDPhoton40_v", inf, outf, sb->branches("hltobject/HLT_HIGEDPhoton40_v"));
  s["ggGED"] = new phoD::skimtree("ggHiNtuplizerGED/EventTree", inf, outf, sb->branches("ggHiNtuplizerGED/EventTree"));
  s["skim"] = new phoD::skimtree("skimanalysis/HltTree", inf, outf, sb->branches("skimanalysis/HltTree"));
  s["hi"] = new phoD::skimtree("hiEvtAnalyzer/HiTree", inf, outf, sb->branches("hiEvtAnalyzer/HiTree"));
  s["forest"] = new phoD::skimtree("HiForest/HiForestInfo", inf, outf, sb->branches("HiForest/HiForestInfo"));
  // s["nt"] = new phoD::skimtree("Dfinder/ntDkpi", inf, outf, sb->branches("Dfinder/ntDkpi"));
  TTree* nt = (TTree*)inf->Get("Dfinder/ntDkpi");
  phoD::dtree* dt = new phoD::dtree(nt);
  phoD::dtree* dt_new = new phoD::dtree(outf, "Dfinder/ntDkpi");

  int pprimaryVertexFilter; s["skim"]->t()->SetBranchAddress("pprimaryVertexFilter", &pprimaryVertexFilter);
  int phfCoincFilter2Th4; s["skim"]->t()->SetBranchAddress("phfCoincFilter2Th4", &phfCoincFilter2Th4);
  int pclusterCompatibilityFilter; s["skim"]->t()->SetBranchAddress("pclusterCompatibilityFilter", &pclusterCompatibilityFilter);
  int hiBin; s["hi"]->t()->SetBranchAddress("hiBin", &hiBin);

  s["forest"]->GetEntry(0);
  s["forest"]->Fill();

  // MVA
  phoD::dmva* mva = new phoD::dmva("mva/");

  int nentries = s["hi"]->nentries();
  for(int i=0; i<nentries; i++)
    {
      if(i%100==0) xjjc::progressbar(i, nentries);
      for(auto& ss : s) { if(ss.first != "forest") { ss.second->GetEntry(i); } }
      dt->GetEntry(i);

      if(!(pprimaryVertexFilter && phfCoincFilter2Th4 && pclusterCompatibilityFilter)) continue;

      for(auto& ss : s) { if(ss.first != "forest") { ss.second->Fill(); } }
      dt_new->ClearDsize();
      for(int j=0; j<dt->Dsize(); j++)
        {
          if(!dt->presel(j)) continue;
          // if(!(fabs(dt->val<float>("Dalpha", j)) < 0.2 && 
          //      ((dt->val<float>("Dpt", j) < 6. && fabs(dt->val<float>("DsvpvDistance", j)/dt->val<float>("DsvpvDisErr", j)) > 4.5 && dt->val<float>("Dchi2cl", j) > 0.15) ||
          //       (dt->val<float>("Dpt", j) > 6. && fabs(dt->val<float>("DsvpvDistance", j)/dt->val<float>("DsvpvDisErr", j)) > 3.5 && dt->val<float>("Dchi2cl", j) > 0.05))
          //      )) continue;

          float mvaval = mva->eval(dt, j, hiBin);
          if(!mva->pass(mvaval, dt->val<float>("Dpt", j), hiBin)) continue;

          dt_new->setBDT(j, mvaval);
          dt_new->Fillall(dt, j);
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
  if(argc==3) { return skim(argv[1], argv[2]); }
  return 1;
}
