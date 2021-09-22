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
  weightdirs[ptbins*absybins]
  xmlnames[ptbins*absybins][methods]
*/
/******************************************/

namespace mytmva
{
  int createmva(TTree* nttree, TFile* outf, std::vector<std::vector<std::string>> xmlnames, bool ishi, int nevt=-1);
  std::string titlecolor = "\e[34;3m", nocolor = "\e[0m", contentcolor = "\e[34m", errorcolor = "\e[31;1m";
  int mvaprob(std::string inputname, std::string treename, bool ishi, std::string outputname, std::vector<std::string> weightdirs,
              int nevt=-1, std::vector<std::string> rootfname = std::vector<std::string>(ptbins.size()*absybins.size()-1, ""));
  int whichbin(float pt, float absy);
}

// ------------------ input file ------------- ntmix ---------- partial output file name ---- weightdir list for ptbins ------ nevent -------- training root files --------
int mytmva::mvaprob(std::string inputname, std::string treename, bool ishi, std::string outputname, std::vector<std::string> weightdirs, int nevt, std::vector<std::string> rootfnames)
{
  std::cout<<std::endl;
  if(weightdirs.size() != mytmva::nptbins*mytmva::nabsybins || weightdirs.size() != rootfnames.size()) 
    {
      std::cout<<__FUNCTION__<<": error: invalid number of weightdirs or rootfnames."<<std::endl;
      return 1;
    }

  // extract info into
  std::vector<std::vector<std::string>> xmlnames(mytmva::nptbins*mytmva::nabsybins);
  for(int j=0; j<mytmva::nabsybins; j++)
    for(int i=0; i<mytmva::nptbins; i++)
      {
        std::string weightdir = weightdirs[j*mytmva::nptbins + i];
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
                xmlnames[j*mytmva::nptbins + i].push_back(entrypath);
                std::cout<<entrypath<<std::endl;
              }
          }
      }

  // training rootfile
  std::vector<std::string> cuts(mytmva::nptbins*mytmva::nabsybins, ""), cutb(mytmva::nptbins*mytmva::nabsybins, ""), varinfo(mytmva::nptbins*mytmva::nabsybins, "");
  for(int j=0; j<mytmva::nabsybins; j++)
    for(int i=0; i<mytmva::nptbins; i++)
      {
        std::string rootfname = rootfnames[j*mytmva::nptbins + i];
        if(rootfname == "")
          {
            std::string reviserootf = xjjc::str_replaceall(weightdirs[j*mytmva::nptbins + i], "dataset/weights/rootfiles_", "rootfiles/");
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
            cuts[j*mytmva::nptbins + i] = *cuts_; cutb[j*mytmva::nptbins + i] = *cutb_; varinfo[j*mytmva::nptbins + i] = *varinfo_;
            rootf->Close();
          }
        else { std::cout<<"\e[33m"<<"==> "<<__FUNCTION__<<": warning: file:"<<rootfname.c_str()<<" doesn't exist. skipped."<<mytmva::nocolor<<std::endl; }
      }

  // input/output file // !! produce a better outputfile name !! todo
  std::cout<<mytmva::titlecolor<<"==> "<<__FUNCTION__<<": input file:"<<mytmva::nocolor<<std::endl<<inputname<<mytmva::nocolor<<std::endl;
  std::string outfname(outputname);
  for(int j=0; j<mytmva::nabsybins; j++)
    for(int i=0; i<mytmva::nptbins; i++)
      {
        std::string weightlabel = xjjc::str_replaceall(xjjc::str_replaceallspecial(weightdirs[j*mytmva::nptbins + i]), "dataset_weights_rootfiles_TMVA_", "");
        outfname += ("_"+xjjc::str_replaceall(weightlabel, "_root", ""));
        break;
      }
  outfname += (std::string(Form("_%dbin", mytmva::nptbins*mytmva::nabsybins))+".root");
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
  for(int j=0; j<mytmva::nabsybins; j++)
    for(int i=0; i<mytmva::nptbins; i++)
      {
        info->Branch(Form("weightdir_pt-%.0f-%.0f_absy-%.0f-%.0f", mytmva::ptbins[i], mytmva::ptbins[i+1], mytmva::absybins[j], mytmva::absybins[j+1]), &(weightdirs[j*mytmva::nptbins + i]));
        info->Branch(Form("cuts_pt-%.0f-%.0f_absy-%.0f-%.0f", mytmva::ptbins[i], mytmva::ptbins[i+1], mytmva::absybins[j], mytmva::absybins[j+1]), &(cuts[j*mytmva::nptbins + i]));
        info->Branch(Form("cutb_pt-%.0f-%.0f_absy-%.0f-%.0f", mytmva::ptbins[i], mytmva::ptbins[i+1], mytmva::absybins[j], mytmva::absybins[j+1]), &(cutb[j*mytmva::nptbins + i]));
        info->Branch(Form("var_pt-%.0f-%.0f_absy-%.0f-%.0f", mytmva::ptbins[i], mytmva::ptbins[i+1], mytmva::absybins[j], mytmva::absybins[j+1]), &(varinfo[j*mytmva::nptbins + i]));
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

  // read methods and variables from weight file
  std::vector<std::string> methods, varnames; 
  std::cout<<mytmva::titlecolor<<"==> "<<__FUNCTION__<<": Found method:"<<mytmva::nocolor<<std::endl;
  const int ii = 0;
  // std::cout<<mytmva::titlecolor<<" => "<<__FUNCTION__<<": "<<Form("pt: %.0f - %.0f, absy: %.0f - %.0f", mytmva::ptbins[i], mytmva::ptbins[i+1], mytmva::absybins[j], mytmva::absybins[j+1])<<mytmva::nocolor<<std::endl;
  for(auto& ww : xmlnames[ii]) // 
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
      methods.push_back(method);
      std::cout<<std::left<<std::setw(10)<<method<<" // "<<fullmethodname<<mytmva::nocolor<<std::endl;
      // variable
      if(!varnames.empty()) continue;
      void* variables = TMVA::gTools().GetChild(rootnode, "Variables");
      UInt_t NVar=0;
      TMVA::gTools().ReadAttr(variables, "NVar", NVar);
      void* var = TMVA::gTools().GetChild(variables, "Variable");
      for(unsigned int k=0;k<NVar;k++)
        {
          std::string varlabel("");
          TMVA::gTools().ReadAttr(var, "Label", varlabel);
          varnames.push_back(varlabel);
          var = TMVA::gTools().GetNextChild(var);
        }
    }
  if(methods.size() <= 0) { std::cout<<__FUNCTION__<<": error: no method is registered."<<std::endl; return 1; }
  // 
  int nmeth = methods.size(), 
    nvar = varnames.size();
  
  std::vector<std::vector<float>> __varval(mytmva::nptbins*mytmva::nabsybins);
  std::vector<TMVA::Reader*> readers(mytmva::nptbins*mytmva::nabsybins);
  for(int j=0; j<mytmva::nabsybins; j++)
    for(int i=0; i<mytmva::nptbins; i++)
      { readers[j*mytmva::nptbins + i] = new TMVA::Reader( "!Color:!Silent" ); }
  std::cout<<mytmva::titlecolor<<"==> "<<__FUNCTION__<<": Add variable:"<<mytmva::nocolor<<std::endl;
  for(int j=0; j<mytmva::nabsybins; j++)
    for(int i=0; i<mytmva::nptbins; i++)
      {
        std::cout<<mytmva::titlecolor<<" => "<<__FUNCTION__<<": "<<Form("pt: %.0f - %.0f, absy: %.0f - %.0f", mytmva::ptbins[j*mytmva::nptbins + i], mytmva::ptbins[i+1], mytmva::absybins[j], mytmva::absybins[j+1])<<mytmva::nocolor<<std::endl;
        __varval[j*mytmva::nptbins + i].resize(nvar, 0);
        for(int vv=0; vv<nvar; vv++)
          {
            std::cout<<std::left<<std::setw(10)<<varnames[vv]<<" // "<<mytmva::findvar(varnames[vv])->var.c_str()<<std::endl;
            readers[j*mytmva::nptbins + i]->AddVariable(mytmva::findvar(varnames[vv])->var.c_str(), &(__varval[j*mytmva::nptbins + i][vv])); 
          }
      }
  std::cout<<mytmva::titlecolor<<"==> "<<__FUNCTION__<<": Book method:"<<mytmva::nocolor<<std::endl;
  for(int j=0; j<mytmva::nabsybins; j++)
    for(int i=0; i<mytmva::nptbins; i++)
      {
        std::cout<<mytmva::titlecolor<<" => "<<__FUNCTION__<<": "<<Form("pt: %.0f - %.0f, absy: %.0f - %.0f", mytmva::ptbins[i], mytmva::ptbins[i+1], mytmva::absybins[j], mytmva::absybins[j+1])<<mytmva::nocolor<<std::endl;
        for(int mm=0; mm<nmeth; mm++)
          {
            std::string methodtag(methods[mm] + " method");
            readers[j*mytmva::nptbins + i]->BookMVA( methodtag.c_str(), xmlnames[j*mytmva::nptbins + i][mm].c_str() ); // ~
          }
      }

  outf->cd();
  if(!outf->cd("dataset")) { outf->mkdir("dataset"); outf->cd("dataset"); }
  std::string varnote;
  for(auto& vv : varnames) varnote += (";"+vv);
  TTree* note = new TTree("weightnote", "");
  note->Branch("varnote", &varnote);
  note->Fill();

  int mvaDsize;
  std::vector<float[MAX_XB]> __mvaval(nmeth);
  outf->cd("dataset");
  TTree* mvatree = new TTree("mva", "");
  mvatree->Branch("mvaDsize", &mvaDsize);
  for(int mm=0; mm<nmeth; mm++) // !! how about different pt has different methods
    { mvatree->Branch(methods[mm].c_str(), __mvaval[mm], Form("%s[mvaDsize]/F", methods[mm].c_str())); }
  bool __mvapref[MAX_XB];
  mvatree->Branch("mvapref", __mvapref, "mvapref[mvaDsize]/O");
  
  std::cout<<mytmva::titlecolor<<"==> "<<__FUNCTION__<<": Filling mva values:"<<mytmva::nocolor<<std::endl;
  outf->cd();
  int nentries = nevt>0&&nevt<values->getnt()->nt()->GetEntries()?nevt:values->getnt()->nt()->GetEntries();
  for(int i=0; i<nentries; i++)
    {
      xjjc::progressslide(i, nentries, 300);
      values->getnt()->nt()->GetEntry(i);

      mvaDsize = values->getnt()->Dsize();
      for(int j=0; j<values->getnt()->Dsize(); j++)
        {
          float pt = values->getnt()->val<float>("Dpt", j),
            absy = fabs(values->getnt()->val<float>("Dy", j));
          int idxpt = mytmva::whichbin(pt, absy); // split pt, y
          if(idxpt < 0) { std::cout<<" \e[31;1merror:\e[0m bad kinematics bin: pt = "<<pt<<", absy = "<<absy<<std::endl; continue; }
          for(int vv=0; vv<nvar; vv++)
            { __varval[idxpt][vv] = values->getval(varnames[vv], j); }
          for(int mm=0; mm<nmeth; mm++)
            {
              std::string methodtag(methods[mm] + " method");
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

#endif
