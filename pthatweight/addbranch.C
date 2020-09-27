#include <iostream>

#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include <TString.h>
#include <TDirectory.h>
#include <string>
#include <regex>

#include "typeinfo.h"

std::string globaltypename;

namespace xjjc
{
  template <typename T>
  void addbranch(std::string inputname, std::string treename, std::string branchname, T branchvalue);
  bool str_isnumber(std::string strs) { return (std::regex_match(strs, std::regex("-?[0-9]+([.][0-9]*)?")) || std::regex_match(strs, std::regex("-?[0-9]*[.][0-9]+"))); }
  bool str_isinteger(std::string strs) { return std::regex_match(strs, std::regex("-?[0-9]+")); }
  std::vector<std::string> str_divide(std::string str, std::string div);
  void progressbar(int index_, int total_, int morespace_=0);
  void progressbar_summary(int total_);
}

template <typename T>
void xjjc::addbranch(std::string inputname, std::string treename, std::string branchname, 
                     T branchvalue)
{
  char warning = 'x';
  std::cout<<std::endl<<" -- Warning..."<<std::endl;
  std::cout<<"*** This will change the file: "<<std::endl<<"  "<<inputname<<std::endl;
  while(warning != 'n' && warning != 'y')
    {
      std::cout<<"*** Do you want to continue? [y/n]"<<std::endl;
      std::cin>>warning;
    }
  if(warning == 'n') { return; }

  std::cout<<std::endl<<" -- Checking branch type"<<std::endl;
  std::cout<<"file: "<<inputname<<std::endl;
  std::cout<<"tree: "<<treename<<std::endl;
  xjjc::typeinfo<T> typebr(globaltypename);
  typebr.printinfo(true, true);

  std::cout<<std::endl<<" -- Building tree"<<std::endl;
  TFile* inf = TFile::Open(inputname.c_str(), "update");
  if(!inf->IsOpen()) { std::cout<<__FUNCTION__<<": error: invalid input."<<std::endl<<"  "<<inputname<<std::endl; return; }
  std::vector<std::string> tdirs = xjjc::str_divide(treename, "/"); 
  std::string trname = tdirs[tdirs.size()-1];
  tdirs.pop_back();
  TTree* tr = (TTree*)inf->Get(treename.c_str());
  bool existtree = (bool)tr;
  inf->cd();
  if(!existtree) 
    { 
      int ndir = 1;
      for(auto& dir : tdirs) 
        { 
          if(!gDirectory->cd(dir.c_str())) 
            { 
              gDirectory->mkdir(dir.c_str()); 
              std::cout<<std::string(ndir*3, '*')<<" TDirectory "<<dir.c_str()<<" is created."<<std::endl;
              gDirectory->cd(dir.c_str()); 
              ndir++;
            }
        }
      tr = new TTree(trname.c_str(), "");
      std::cout<<"TTree "<<treename<<" is created."<<std::endl;
    }
  else { std::cout<<"TTree "<<treename<<" is opened."<<std::endl; }
  inf->cd();

  std::cout<<std::endl<<" -- Building and filling branch"<<std::endl;
  T brval;
  TBranch* br = tr->Branch(branchname.c_str(), &brval);
  brval = branchvalue;
  if(existtree)
    {
      int nentries = tr->GetEntries();
      for(int i=0; i<nentries; i++) 
        {
          if(i%1000==0) xjjc::progressbar(i, nentries);
          br->Fill();
        }
      xjjc::progressbar_summary(nentries);
    }
  else { tr->Fill(); }
  inf->cd();
  for(auto& dir : tdirs) { if(!gDirectory->cd(dir.c_str())) { std::cout<<__FUNCTION__<<": error: did not manage to mkdir at beginning."<<std::endl; return; } }
  tr->Write("", TObject::kOverwrite);
  inf->Close();

  std::cout<<std::endl<<" -- End"<<std::endl<<std::endl;
}

int main(int argc, char* argv[])
{
  if(argc == 6 || argc == 5)
    {
      int val_int = atoi(argv[4]);
      float val_float = atof(argv[4]);
      double val_double = atof(argv[4]);
      TString val_string = TString(argv[4]);

      std::string thistype, thisvalue(argv[4]);
      if(argc==6) { thistype = std::string(argv[5]); }
      if(argc==5) 
        {
          if(!xjjc::str_isnumber(thisvalue)) { thistype = "TString"; }
          else if(xjjc::str_isinteger(thisvalue)) { thistype = "int"; }
          else { thistype = "float"; }
        }

      globaltypename = thistype;

      //
      if(thistype=="Int_t" || thistype=="int" || thistype=="Bool_t" || thistype=="bool")
        {
          if(!xjjc::str_isinteger(thisvalue)) { std::cout<<__FUNCTION__<<": inconsistent value and type: ("<<thistype<<")"<<thisvalue<<std::endl; return 2; }
          xjjc::addbranch(argv[1], argv[2], argv[3], val_int); return 0;
        }
      if(thistype=="Float_t" || thistype=="float")
        {
          if(!xjjc::str_isnumber(thisvalue)) { std::cout<<__FUNCTION__<<": inconsistent value and type: ("<<thistype<<")"<<thisvalue<<std::endl; return 2; }
          xjjc::addbranch(argv[1], argv[2], argv[3], val_float); return 0;
        }
      if(thistype=="Double_t" || thistype=="double")
        {
          if(!xjjc::str_isnumber(thisvalue)) { std::cout<<__FUNCTION__<<": inconsistent value and type: ("<<thistype<<")"<<thisvalue<<std::endl; return 2; }
          xjjc::addbranch(argv[1], argv[2], argv[3], val_double); return 0;
        }
      if(thistype=="TString")
        {
          xjjc::addbranch(argv[1], argv[2], argv[3], val_string); return 0;
        }
      else 
        {
          std::cout<<__FUNCTION__<<": invalid type: "<<thistype<<std::endl;
          return 2;
        }
    }

  std::cout<<__FUNCTION__<<": ./addbranch.exe [inputfile] [treename] [branchname] [value] ([type])"<<std::endl;
  return 1;
}

std::vector<std::string> xjjc::str_divide(std::string str, std::string div)
{
  std::vector<std::string> token;
  size_t lastpos = 0;
  size_t pos = str.find(div, 0);
  while(pos != std::string::npos)
    {
      std::string thistoken = str.substr(lastpos, pos-lastpos);
      token.push_back(thistoken);
      lastpos = pos+1;
      pos = str.find(div, pos+1);
    }
  if(lastpos != 0) { token.push_back(str.substr(lastpos, str.size()-lastpos)); }
  return token;
}

void xjjc::progressbar(int index_, int total_, int morespace_/*=0*/)
{
  std::cout<<std::setiosflags(std::ios::left)<<"  [ \033[1;36m"<<std::setw(10+morespace_)<<index_<<"\033[0m"<<" / "<<std::setw(10+morespace_)<<total_<<" ] "<<"\033[1;36m"<<(int)(100.*index_/total_)<<"%\033[0m"<<"\r"<<std::flush;
}

void xjjc::progressbar_summary(int total_)
{
  std::cout<<std::endl<<"  Processed "<<"\033[1;31m"<<total_<<"\033[0m event(s)."<<std::endl;
}
