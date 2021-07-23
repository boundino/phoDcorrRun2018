#ifndef __MVAPROD_H_
#define __MVAPROD_H_

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

#include <experimental/filesystem>

#include <TFile.h>
#include <TTree.h>
#include <TSystem.h>
#include "TMVA/Reader.h"
#include "TMVA/Tools.h"

#include "TMVAClassification.h"
#include "xjjcuti.h"

#ifndef MAX_XB
#define MAX_XB       20000
#endif

namespace fs = std::experimental::filesystem;

/******************************************/
/*
 weightdirs[ptbins]
 xmlnames[ptbins][methods]
*/
/******************************************/

namespace mytmva
{
  int createmva(TTree* nttree, TFile* outf, std::vector<std::vector<std::string>> xmlnames, bool ishi, int nevt=-1);
  std::string titlecolor = "\e[34;3m", nocolor = "\e[0m", contentcolor = "\e[34m", errorcolor = "\e[31;1m";
  int mvaprob(std::string inputname, std::string treename, bool ishi, std::string outputname, std::vector<std::string> weightdirs,
              int nevt=-1, std::vector<std::string> rootfname = std::vector<std::string>(ptbins.size()-1, ""));
  int whichbin(float pt);
}

// ------------------ input file ------------- ntmix ---------- partial output file name ---- weightdir list for ptbins ------ nevent -------- training root files --------
int mytmva::mvaprob(std::string inputname, std::string treename, bool ishi, std::string outputname, std::vector<std::string> weightdirs, int nevt, std::vector<std::string> rootfnames)
{
  std::cout<<std::endl;
  if(weightdirs.size() != mytmva::nptbins || weightdirs.size() != rootfnames.size()) 
    {
      std::cout<<__FUNCTION__<<": error: invalid number of weightdirs or rootfnames."<<std::endl;
      return 1;
    }

  // extract info into
  std::vector<std::vector<std::string>> xmlnames(mytmva::nptbins);
  for(int i=0; i<mytmva::nptbins; i++)
    {
      std::string weightdir = weightdirs[i];
      if(weightdir.back() == '/') { weightdir.pop_back(); }
      std::cout<<mytmva::titlecolor<<"==> "<<__FUNCTION__<<": directory of weight files:"<<mytmva::nocolor<<std::endl;
      std::cout<<weightdir<<std::endl;

      // resolve methods
      std::cout<<mytmva::titlecolor<<"==> "<<__FUNCTION__<<": found weight files:"<<mytmva::nocolor<<std::endl;
      for (const auto & entry : fs::directory_iterator(weightdir))
        {
          std::string entrypath(entry.path());
          if(xjjc::str_contains(entrypath, ".weights.xml")) 
            {
              xmlnames[i].push_back(entrypath);
              std::cout<<entrypath<<std::endl;
            }
        }
    }

  // training rootfile
  std::vector<std::string> cuts(mytmva::nptbins, ""), cutb(mytmva::nptbins, ""), varinfo(mytmva::nptbins, "");
  for(int i=0; i<mytmva::nptbins; i++)
    {
      std::string rootfname = rootfnames[i];
      if(rootfname == "")
        {
          std::string reviserootf = xjjc::str_replaceall(weightdirs[i], "dataset/weights/rootfiles_", "rootfiles/");
          reviserootf = xjjc::str_replaceall(reviserootf, "_root", ".root");
          rootfname = reviserootf;
        }

      bool findrootf = !gSystem->AccessPathName(rootfname.c_str());
      if(findrootf)
        {
          TString *cuts_ = 0, *cutb_ = 0; std::string *varinfo_ = 0;
          TFile* rootf = TFile::Open(rootfname.c_str());
          std::cout<<mytmva::titlecolor<<"==> "<<__FUNCTION__<<": opening file:"<<mytmva::nocolor<<std::endl<<rootfname<<mytmva::nocolor<<std::endl;
          if(!rootf) { std::cout<<mytmva::errorcolor<<"==> "<<__FUNCTION__<<": error: file is not opened."<<mytmva::nocolor<<std::endl; return 1; }
          TTree* rinfo = (TTree*)rootf->Get("dataset/tmvainfo");
          if(!rinfo) { std::cout<<mytmva::errorcolor<<"==> "<<__FUNCTION__<<": error: tree is not opened."<<mytmva::nocolor<<std::endl; return 1; }
          rinfo->SetBranchAddress("cuts", &cuts_);
          rinfo->SetBranchAddress("cutb", &cutb_);
          rinfo->SetBranchAddress("var", &varinfo_);
          // std::cout<<mytmva::titlecolor<<std::endl; rinfo->Show(0); std::cout<<mytmva::nocolor<<std::endl;
          std::cout<<mytmva::titlecolor<<"==> "<<__FUNCTION__<<": mva info:"<<mytmva::nocolor<<std::endl;
          rinfo->Show(0); std::cout<<std::endl;
          rinfo->GetEntry(0);
          cuts[i] = *cuts_; cutb[i] = *cutb_; varinfo[i] = *varinfo_;
          rootf->Close();
        }
      else { std::cout<<"\e[33m"<<"==> "<<__FUNCTION__<<": warning: file:"<<rootfname.c_str()<<" doesn't exist. skipped."<<mytmva::nocolor<<std::endl; }
    }

  // input/output file // !! produce a better outputfile name !! todo
  std::cout<<mytmva::titlecolor<<"==> "<<__FUNCTION__<<": input file:"<<mytmva::nocolor<<std::endl<<inputname<<mytmva::nocolor<<std::endl;
  std::string outfname(outputname);
  for(int i=0; i<mytmva::nptbins; i++)
    {
      std::string weightlabel = xjjc::str_replaceall(xjjc::str_replaceallspecial(weightdirs[i]), "dataset_weights_rootfiles_TMVA_", "");
      outfname += ("_"+xjjc::str_replaceall(weightlabel, "_root", ""));
      break;
    }
  outfname += (std::string(Form("_%dbin", mytmva::nptbins))+".root");
  std::cout<<mytmva::titlecolor<<"==> "<<__FUNCTION__<<": output file:"<<mytmva::nocolor<<std::endl<<outfname<<mytmva::nocolor<<std::endl;
  if(std::experimental::filesystem::exists(outfname)) { std::cout<<mytmva::errorcolor<<"==> "<<__FUNCTION__<<": warning: output file already exists."<<mytmva::nocolor<<std::endl; }
  std::cout<<"==> "<<__FUNCTION__<<": warning: application of mva values will take long time. would you want to continue? [y/n]"<<std::endl; char ans='x';
  while(ans!='y' && ans!='n') { std::cin>>ans; }
  if(ans=='n') return 0;
  gSystem->Exec(Form("rsync --progress %s %s", inputname.c_str(), outfname.c_str()));

  // fill cut info
  TFile* inf = TFile::Open(inputname.c_str());
  TTree* nttree = (TTree*)inf->Get(treename.c_str());
  TFile* outf = TFile::Open(outfname.c_str(), "update");
  outf->mkdir("dataset");
  outf->cd("dataset");
  TTree* info = new TTree("tmvainfo", "TMVA info");
  for(int i=0; i<mytmva::nptbins; i++)
    {
      info->Branch(Form("weightdir_%.0f_%.0f", mytmva::ptbins[i], mytmva::ptbins[i+1]), &(weightdirs[i]));
      info->Branch(Form("cuts_%.0f_%.0f", mytmva::ptbins[i], mytmva::ptbins[i+1]), &(cuts[i]));
      info->Branch(Form("cutb_%.0f_%.0f", mytmva::ptbins[i], mytmva::ptbins[i+1]), &(cutb[i]));
      info->Branch(Form("var_%.0f_%.0f", mytmva::ptbins[i], mytmva::ptbins[i+1]), &(varinfo[i]));
    }
  info->Fill();
  info->Write("", TObject::kOverwrite);

  outf->cd();
  mytmva::createmva(nttree, outf, xmlnames, ishi, nevt);
  std::cout<<mytmva::titlecolor<<"==> "<<__FUNCTION__<<": output file:"<<mytmva::nocolor<<std::endl<<outfname<<mytmva::nocolor<<std::endl;

  return 0;
}

int mytmva::createmva(TTree* nttree, TFile* outf, std::vector<std::vector<std::string>> xmlnames, bool ishi, int nevt)
{
  std::cout<<mytmva::titlecolor<<"==> "<<__FUNCTION__<<": Set branch address:"<<mytmva::nocolor<<std::endl;
  mytmva::varval* values = new mytmva::varval(nttree, ishi);
  if(!values->isvalid()) { return 1; }

  // read weight file
  std::vector<std::vector<std::string>> methods(mytmva::nptbins); 
  std::vector<std::map<std::string, std::vector<std::string>>> varlabels(mytmva::nptbins), speclabels(mytmva::nptbins);
  std::cout<<mytmva::titlecolor<<"==> "<<__FUNCTION__<<": Found method:"<<mytmva::nocolor<<std::endl;
  for(int i=0; i<mytmva::nptbins; i++)
    {
      std::cout<<mytmva::titlecolor<<" => "<<__FUNCTION__<<": "<<Form("%.0f - %.0f", mytmva::ptbins[i], mytmva::ptbins[i+1])<<mytmva::nocolor<<std::endl;
      for(auto& ww : xmlnames[i]) // 
        {
          const char* filename = ww.c_str();
          void *doc = TMVA::gTools().xmlengine().ParseFile(filename,TMVA::gTools().xmlenginebuffersize());
          void* rootnode = TMVA::gTools().xmlengine().DocGetRootElement(doc); //node "MethodSetup"
          // method
          std::string fullmethodname("");
          TMVA::gTools().ReadAttr(rootnode, "Method", fullmethodname);
          std::string method = fullmethodname;
          std::size_t found = fullmethodname.find("::");
          method.erase(0, found+2);
          methods[i].push_back(method);
          std::cout<<std::left<<std::setw(10)<<method<<" // "<<fullmethodname<<mytmva::nocolor<<std::endl;
          // variable
          void* variables = TMVA::gTools().GetChild(rootnode, "Variables");
          UInt_t NVar=0;
          TMVA::gTools().ReadAttr(variables, "NVar", NVar);
          void* var = TMVA::gTools().GetChild(variables, "Variable");
          for(unsigned int k=0;k<NVar;k++)
            {
              std::string varlabel("");
              TMVA::gTools().ReadAttr(var, "Label", varlabel);
              varlabels[i][method].push_back(varlabel);
              var = TMVA::gTools().GetNextChild(var);
            }
          // spectator
          void* spectators = TMVA::gTools().GetChild(rootnode, "Spectators");
          UInt_t NSpec=0;
          TMVA::gTools().ReadAttr(spectators, "NSpec", NSpec);
          void* spec = TMVA::gTools().GetChild(spectators, "Spectator");
          for(unsigned int k=0;k<NSpec;k++)
            {
              std::string speclabel("");
              TMVA::gTools().ReadAttr(spec, "Label", speclabel);
              speclabels[i][method].push_back(speclabel);
              spec = TMVA::gTools().GetNextChild(spec);
            }
        }
      if(methods[i].size() <= 0) { std::cout<<__FUNCTION__<<": error: no method is registered."<<std::endl; return 1; }
    }
  // 
  std::vector<int> nmeth(mytmva::nptbins);
  std::vector<std::vector<std::string>> varnames(mytmva::nptbins);
  std::vector<int> nvar(mytmva::nptbins);
  std::vector<std::vector<std::string>> specnames(mytmva::nptbins);
  std::vector<int> nspec(mytmva::nptbins);
  for(int i=0; i<mytmva::nptbins; i++)
    {
      nmeth[i] = methods[i].size();
      varnames[i] = varlabels[i][methods[i][0]];
      specnames[i] = speclabels[i][methods[i][0]];
      nvar[i] = varnames[i].size();
      nspec[i] = specnames[i].size();
      for(auto& mm : methods[i])
        {
          // check variable
          if(varlabels[i][mm].size() != nvar[i])
            { std::cout<<__FUNCTION__<<": error: inconsistent variable number among methods."<<std::endl; return 1; }
          for(int vv=0; vv<nvar[i]; vv++)
            {
              if(varlabels[i][mm].at(vv) != varnames[i][vv]) 
                { std::cout<<__FUNCTION__<<": error: inconsistent variable among methods."<<std::endl; return 1; }
            }
          // check spectator
          if(speclabels[i][mm].size() != nspec[i])
            { std::cout<<__FUNCTION__<<": error: inconsistent spectator number among methods."<<std::endl; return 1; }
          for(int vv=0; vv<nspec[i]; vv++)
            {
              if(speclabels[i][mm].at(vv) != specnames[i][vv]) 
                { std::cout<<__FUNCTION__<<": error: inconsistent spectator among methods."<<std::endl; return 1; }
            }
        }
    }
  
  std::vector<std::string> varnote(mytmva::nptbins, "");
  std::vector<std::vector<float>> __varval(mytmva::nptbins), __specval(mytmva::nptbins);
  std::vector<TMVA::Reader*> readers(mytmva::nptbins);
  for(int i=0; i<mytmva::nptbins; i++)
    { readers[i] = new TMVA::Reader( "!Color:!Silent" ); }
  std::cout<<mytmva::titlecolor<<"==> "<<__FUNCTION__<<": Add variable:"<<mytmva::nocolor<<std::endl;
  for(int i=0; i<mytmva::nptbins; i++)
    {
      std::cout<<mytmva::titlecolor<<" => "<<__FUNCTION__<<": "<<Form("%.0f - %.0f", mytmva::ptbins[i], mytmva::ptbins[i+1])<<mytmva::nocolor<<std::endl;
      __varval[i].resize(nvar[i], 0);
      for(int vv=0; vv<nvar[i]; vv++)
        {
          std::cout<<std::left<<std::setw(10)<<varnames[i][vv]<<" // "<<mytmva::findvar(varnames[i][vv])->var.c_str()<<std::endl;
          readers[i]->AddVariable(mytmva::findvar(varnames[i][vv])->var.c_str(), &(__varval[i][vv])); 
          varnote[i] += (";"+varnames[i][vv]);
        }
    }
  std::cout<<mytmva::titlecolor<<"==> "<<__FUNCTION__<<": Add spectator:"<<mytmva::nocolor<<std::endl;
  for(int i=0; i<mytmva::nptbins; i++)
    {
      std::cout<<mytmva::titlecolor<<" => "<<__FUNCTION__<<": "<<Form("%.0f - %.0f", mytmva::ptbins[i], mytmva::ptbins[i+1])<<mytmva::nocolor<<std::endl;
      __specval[i].resize(nspec[i], 0);
      for(int vv=0; vv<nspec[i]; vv++)
        {
          std::cout<<std::left<<std::setw(10)<<specnames[i][vv]<<" // "<<mytmva::findvar(specnames[i][vv])->var.c_str()<<std::endl;
          readers[i]->AddSpectator(mytmva::findvar(specnames[i][vv])->var.c_str(), &(__specval[i][vv])); 
          // varnote += (";"+specnames[vv]);
        }
    }
  std::cout<<mytmva::titlecolor<<"==> "<<__FUNCTION__<<": Book method:"<<mytmva::nocolor<<std::endl;
  for(int i=0; i<mytmva::nptbins; i++)
    {
      std::cout<<mytmva::titlecolor<<" => "<<__FUNCTION__<<": "<<Form("%.0f - %.0f", mytmva::ptbins[i], mytmva::ptbins[i+1])<<mytmva::nocolor<<std::endl;
      for(int mm=0; mm<nmeth[i]; mm++)
        {
          std::string methodtag(methods[i][mm] + " method");
          readers[i]->BookMVA( methodtag.c_str(), xmlnames[i][mm].c_str() ); // ~
        }
    }

  outf->cd();
  if(!outf->cd("dataset")) { outf->mkdir("dataset"); outf->cd("dataset"); }
  TTree* note = new TTree("weightnote", "");
  note->Branch("varnote", &varnote);
  note->Fill();

  int mvaBsize;
  std::vector<float[MAX_XB]> __mvaval(nmeth[0]);
  outf->cd("dataset");
  TTree* mvatree = new TTree("mva", "");
  mvatree->Branch("mvaBsize", &mvaBsize);
  for(int mm=0; mm<nmeth[0]; mm++) // !! how about different pt has different methods
    { mvatree->Branch(methods[0][mm].c_str(), __mvaval[mm], Form("%s[mvaBsize]/F", methods[0][mm].c_str())); }
  bool __mvapref[MAX_XB];
  mvatree->Branch("mvapref", __mvapref, "mvapref[mvaBsize]/O");
  
  std::cout<<mytmva::titlecolor<<"==> "<<__FUNCTION__<<": Filling mva values:"<<mytmva::nocolor<<std::endl;
  outf->cd();
  int nentries = nevt>0&&nevt<values->getnt()->nt()->GetEntries()?nevt:values->getnt()->nt()->GetEntries();
  for(int i=0; i<nentries; i++)
    {
      if(i%300==0) xjjc::progressbar(i, nentries);
      values->getnt()->nt()->GetEntry(i);

      mvaBsize = values->getnt()->Dsize();
      for(int j=0; j<values->getnt()->Dsize(); j++)
        {
          float jpt = values->getnt()->val<float>("Dpt", j);
          int idxpt = mytmva::whichbin(jpt); // split pt
          for(int vv=0; vv<nvar[idxpt]; vv++)
            { __varval[idxpt][vv] = values->getval(varnames[idxpt][vv], j); }
          for(int vv=0; vv<nspec[idxpt]; vv++)
            { __specval[idxpt][vv] = values->getval(specnames[idxpt][vv], j); }
          for(int mm=0; mm<nmeth[idxpt]; mm++)
            {
              std::string methodtag(methods[idxpt][mm] + " method");
              __mvaval.at(mm)[j] = readers[idxpt]->EvaluateMVA(methodtag.c_str());
            }
          __mvapref[j] = values->getnt()->presel(j);
        }
      outf->cd("dataset"); 
      mvatree->Fill(); 
    }
  xjjc::progressbar_summary(nentries);

  outf->cd("dataset");
  mvatree->Write("", TObject::kOverwrite);
  // outf->Write();
  outf->cd();
  outf->Close();

  return 0;
}

int mytmva::whichbin(float pt)
{
  std::vector<float> bins(mytmva::ptbins);
  if(bins[mytmva::nptbins] < 0) { bins[mytmva::nptbins] = 1.e+10; }
  int idx = -1;
  for(int i=0; i<bins.size(); i++)
    {
      if(pt < bins[i]) break;
      idx = i;
    }
  if(idx < 0) idx = 0;
  if(idx == bins.size()-1) idx = bins.size()-2;
  return idx;
}

#endif
