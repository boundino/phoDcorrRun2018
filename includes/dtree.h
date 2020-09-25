#include <TTree.h>
#include <TDirectory.h>

#include <vector>
#include <map>
#include <string>
#include <type_traits>

#include "xjjcuti.h"

#define MAX_XB       20000

namespace phoD
{
  class dtree
  {
  public:
    dtree(TTree* nt);
    dtree(TFile* outf, std::string name);
    TTree* nt() { return nt_; }
    template<typename T> T val(std::string br, int j);
    int GetEntries() { return nt_->GetEntries(); }
    void GetEntry(int i) { nt_->GetEntry(i); }
    int Dsize() { return Dsize_; }
    bool presel(int j);

    void ClearDsize() { Dsize_ = 0; }
    void Fillall(dtree* nt, int j);
    void Fill() { dr_->cd(); nt_->Fill(); }
  private:
    TTree* nt_;
    TDirectory* dr_;
    bool newtree_;
    void setbranchaddress();
    void branch();
    int Dsize_;
    std::vector<std::string> tbvf_ = {
      "Dmass",
      "Dpt",
      "Deta",
      "Dphi",
      "Dy",
      "Dchi2cl",
      "Ddtheta",
      "Dalpha",
      "DsvpvDistance",
      "DsvpvDisErr",
      "DsvpvDistance_2D",
      "DsvpvDisErr_2D",
      "Dtrk1Pt",
      "Dtrk2Pt",
      "Dtrk1PtErr",
      "Dtrk2PtErr",
      "Dtrk1Eta",
      "Dtrk2Eta",
      "Dtrk1Dz1",
      "Dtrk2Dz1",
      "Dtrk1DzError1",
      "Dtrk2DzError1",
      "Dtrk1Dxy1",
      "Dtrk2Dxy1",
      "Dtrk1DxyError1",
      "Dtrk2DxyError1",
      "Dtrk1PixelHit",
      "Dtrk2PixelHit",
      "Dtrk1StripHit",
      "Dtrk2StripHit",
      "Dtrk1nStripLayer",
      "Dtrk2nStripLayer",
      "Dtrk1nPixelLayer",
      "Dtrk2nPixelLayer",
      "Dtrk1Chi2ndf",
      "Dtrk2Chi2ndf"
    };
    std::vector<std::string> tbvi_ = {
    };
    std::vector<std::string> tbvo_ = {
      "Dtrk1highPurity",
      "Dtrk2highPurity"
    };

    std::map<std::string, float*> bvf_;
    std::map<std::string, int*> bvi_;
    std::map<std::string, bool*> bvo_;
  };
}

phoD::dtree::dtree(TTree* nt) : nt_(nt)
{
  std::cout<<"\e[32;1m -- "<<__PRETTY_FUNCTION__<<std::endl;
  for(auto& i : tbvf_) { bvf_[i] = new float[MAX_XB]; }
  for(auto& i : tbvi_) { bvi_[i] = new int[MAX_XB]; }
  for(auto& i : tbvo_) { bvo_[i] = new bool[MAX_XB]; }

  // newtree_ = !nt_->FindBranch("Dsize");
  newtree_ = false;

  nt_->SetBranchStatus("*", 0);
  nt_->SetBranchStatus("Dsize", 1);
  for(auto& b : tbvf_) nt_->SetBranchStatus(b.c_str(), 1);
  for(auto& b : tbvi_) nt_->SetBranchStatus(b.c_str(), 1);
  for(auto& b : tbvo_) nt_->SetBranchStatus(b.c_str(), 1);

  setbranchaddress();
}

phoD::dtree::dtree(TFile* outf, std::string name)
{
  for(auto& i : tbvf_) { bvf_[i] = new float[MAX_XB]; }
  for(auto& i : tbvi_) { bvi_[i] = new int[MAX_XB]; }
  for(auto& i : tbvo_) { bvo_[i] = new bool[MAX_XB]; }

  newtree_ = true;
 
  std::vector<std::string> p = xjjc::str_divide(name, "/");  
  if(p.size() > 1)
    { dr_ = outf->mkdir(p[0].c_str()); }
  else { dr_ = outf; }
  dr_->cd();
  nt_ = new TTree(p.back().c_str(), "");
  branch();
  outf->cd();
}

void phoD::dtree::branch()
{
  nt_->Branch("Dsize", &Dsize_, "Dsize/I");
  for(auto& b : tbvf_) nt_->Branch(b.c_str(), bvf_[b], Form("%s[Dsize]/F", b.c_str()));
  for(auto& b : tbvi_) nt_->Branch(b.c_str(), bvi_[b], Form("%s[Dsize]/I", b.c_str()));
  for(auto& b : tbvo_) nt_->Branch(b.c_str(), bvo_[b], Form("%s[Dsize]/O", b.c_str()));
}

void phoD::dtree::setbranchaddress()
{
  nt_->SetBranchAddress("Dsize", &Dsize_);
  for(auto& b : tbvf_) nt_->SetBranchAddress(b.c_str(), bvf_[b]);
  for(auto& b : tbvi_) nt_->SetBranchAddress(b.c_str(), bvi_[b]);
  for(auto& b : tbvo_) nt_->SetBranchAddress(b.c_str(), bvo_[b]);
}

template<typename T> T phoD::dtree::val(std::string br, int j)
{
  if(std::is_same<T, float>::value) { return bvf_[br][j]; }
  if(std::is_same<T, int>::value) { return bvi_[br][j]; }
  if(std::is_same<T, bool>::value) { return bvo_[br][j]; }
  return (T)0;
}

void phoD::dtree::Fillall(dtree* nt, int j)
{
  for(auto& b : tbvf_) bvf_[b][Dsize_] = nt->val<float>(b, j);
  for(auto& b : tbvi_) bvi_[b][Dsize_] = nt->val<int>(b, j);
  for(auto& b : tbvo_) bvo_[b][Dsize_] = nt->val<bool>(b, j);
  Dsize_++;
}

bool phoD::dtree::presel(int j)
{
  if(
     bvf_["Dtrk1Pt"][j] > 1.0 && bvf_["Dtrk2Pt"][j] > 1.0 && fabs(bvf_["Dtrk1Eta"][j]) < 2.4 && fabs(bvf_["Dtrk2Eta"][j]) < 2.4 &&
     bvo_["Dtrk1highPurity"][j] && bvo_["Dtrk2highPurity"][j] && 
     (bvf_["Dtrk1PixelHit"][j]+bvf_["Dtrk1StripHit"][j]) >= 11 && (bvf_["Dtrk2PixelHit"][j]+bvf_["Dtrk2StripHit"][j]) >= 11 && 
     fabs(bvf_["Dtrk1PtErr"][j]/bvf_["Dtrk1Pt"][j]) < 0.1 && fabs(bvf_["Dtrk2PtErr"][j]/bvf_["Dtrk2Pt"][j]) < 0.1 && 
     (bvf_["Dtrk1Chi2ndf"][j]/(bvf_["Dtrk1nStripLayer"][j]+bvf_["Dtrk1nPixelLayer"][j])) < 0.18 && (bvf_["Dtrk2Chi2ndf"][j]/(bvf_["Dtrk2nStripLayer"][j]+bvf_["Dtrk2nPixelLayer"][j])) < 0.18
     ) return true;
  return false;
}
