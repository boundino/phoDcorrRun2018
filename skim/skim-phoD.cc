#include <TFile.h>
#include <TTree.h>

#include <string>
#include <map>

#include "skimbranch.h"
#include "dtree.h"
#include "ptree.h"
#include "dmva.h"
#include "xjjcuti.h"

int skim(std::string inputname, std::string outputname,
         int ishi, int evtfilt, int hltfilt, int mvafilt,
         float phoEtCut=35., bool removeevent=true)
{
  TFile* inf = TFile::Open(inputname.c_str());
  TFile* outf = new TFile(outputname.c_str(), "recreate");

  phoD::skimbranch* sb = new phoD::skimbranch(ishi);
  std::map<std::string, phoD::skimtree*> s;
  s["hlt"] = new phoD::skimtree("hltanalysis/HltTree", inf, outf, sb->branches("hltanalysis/HltTree"));
  // s["ggGED"] = new phoD::skimtree("ggHiNtuplizerGED/EventTree", inf, outf, sb->branches("ggHiNtuplizerGED/EventTree"));
  s["skim"] = new phoD::skimtree("skimanalysis/HltTree", inf, outf, sb->branches("skimanalysis/HltTree"));
  s["hi"] = new phoD::skimtree("hiEvtAnalyzer/HiTree", inf, outf, sb->branches("hiEvtAnalyzer/HiTree"));
  s["forest"] = new phoD::skimtree("HiForest/HiForestInfo", inf, outf, sb->branches("HiForest/HiForestInfo"));
  s["ntGen"] = new phoD::skimtree("Dfinder/ntGen", inf, outf, sb->branches("Dfinder/ntGen"));
  phoD::dtree* dt = new phoD::dtree((TTree*)inf->Get("Dfinder/ntDkpi"), ishi);
  phoD::dtree* dt_new = new phoD::dtree(outf, "Dfinder/ntDkpi", ishi);
  phoD::ptree* pt = new phoD::ptree((TTree*)inf->Get("ggHiNtuplizerGED/EventTree"), ishi);
  phoD::ptree* pt_new = new phoD::ptree(outf, "ggHiNtuplizerGED/EventTree", ishi, pt->isMC(), pt->nt());

  // hiBin
  int hiBin = -1;
  if(ishi) { s["hi"]->t()->SetBranchAddress("hiBin", &hiBin); }
  // event filter
  std::map<std::string, std::map<bool, std::vector<std::string>>> path;
  path["skim"][true] = std::vector<std::string>{"pprimaryVertexFilter", "phfCoincFilter2Th4", "pclusterCompatibilityFilter"}; // HI
  path["skim"][false] = std::vector<std::string>{"pBeamScrapingFilter", "pPAprimaryVertexFilter"}; // pp
  path["hlt"][true] = std::vector<std::string>{"HLT_HIGEDPhoton40_v1"}; // HI
  path["hlt"][false] = std::vector<std::string>{"HLT_HIPhoton40_HoverELoose_v1"}; // pp

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
      xjjc::progressslide(i, nentries, 100);
      for(auto& ss : s) { if(ss.first != "forest") { ss.second->GetEntry(i); } }
      dt->GetEntry(i);
      pt->GetEntry(i);

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

      // Egamma
      pt_new->ClearnPhoEleMC();
      // fill photon
      for(int j=0; j<pt->nPho(); j++)
        { 
          bool fill = pt->val<float>("phoEt", j) > phoEtCut;
          if(!fill) continue;
          pt_new->Fillall("pho", pt, j);
          pt_new->nPhopp();
        }
      // fill electron
      for(int j=0; j<pt->nEle(); j++)
        {
          bool fill = pt->val<float>("elePt", j) > phoEtCut;
          if(!fill) continue;
          pt_new->Fillall("ele", pt, j);
          pt_new->nElepp();
        }
      // fill gen
      if(pt->isMC())
        {
          for(int j=0; j<pt->nMC(); j++)
            {
              bool fill = true;
              if(!fill) continue;
              pt_new->Fillall("mc", pt, j);
              pt_new->nMCpp();
            }
        }
      if(removeevent && pt_new->nPho()==0 && pt_new->nEle()==0 && pt_new->nMC()==0) continue;

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
                    // std::cout<<j<<" "<<mvaval<<" (";
                    // std::cout<<mva->pass(mvaval, dt->val<float>("Dpt", j), hiBin);
                    // std::cout<<")"<<std::endl;
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

      // ---------------------------- Fill ----------------------------
      for(auto& ss : s) { if(ss.first != "forest") { ss.second->Fill(); } }
      pt_new->Fill();
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
