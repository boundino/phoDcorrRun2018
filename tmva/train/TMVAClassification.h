#ifndef _TMVACLASSIFICATION_H_
#define _TMVACLASSIFICATION_H_

#include <string>
#include <vector>
#include <TString.h>
#include <TRandom.h>

#include "xjjcuti.h"
#include "dtree.h"

namespace mytmva
{
  std::vector<float> ptbins({2., 9999.});
  int nptbins = ptbins.size()-1;
  struct tmvavar
  {
    std::string varname;
    std::string vartex;
    std::string var;
    std::string cutsign;
    float varmin;
    float varmax;
    tmvavar(const std::string& varname_, const std::string& var_, const std::string& cutsign_, const std::string& vartex_, const float& varmin_, const float& varmax_) 
      : varname(varname_), var(var_), cutsign(cutsign_), vartex(vartex_), varmin(varmin_), varmax(varmax_) { ; }
  };

  const std::vector<mytmva::tmvavar> varlist = {
    /*0: */ mytmva::tmvavar("Dmass"         , "Dmass"                                                , ""     , "m_{#mu#mu#pi#pi}"                , 3.6  , 4.0) ,
    /*1: */ mytmva::tmvavar("Dchi2cl"       , "Dchi2cl"                                              , "FMax" , "vertex #chi^{2} prob"            , 0    , 1)   ,
    /*2: */ mytmva::tmvavar("Ddtheta"       , "Ddtheta"                                              , "FMin" , "#theta"                          , 0    , 3.2) ,
    /*3: */ mytmva::tmvavar("DdthetaBScorr" , "DdthetaBScorr"                                        , "FMin" , "#theta (BS)"                     , 0    , 3.2) ,
    /*4: */ mytmva::tmvavar("dls2D"         , "dls2D := TMath::Abs(DsvpvDistance_2D/DsvpvDisErr_2D)" , "FMax" , "l_{xy}/#sigma(l_{xy})"           , 0    , 10)  ,
    /*5: */ mytmva::tmvavar("Dtrk1Pt"       , "Dtrk1Pt"                                              , "FMax" , "trk_{1} p_{T} (GeV/c)"           , 0    , 10)  ,
    /*6: */ mytmva::tmvavar("Dtrk2Pt"       , "Dtrk2Pt"                                              , "FMax" , "trk_{2} p_{T} (GeV/c)"           , 0    , 10)  ,
    /*7: */ mytmva::tmvavar("Dtrk1Eta"      , "Dtrk1Eta"                                             , ""     , "trk_{1} #eta"                    , -2   , 2)   ,
    /*8: */ mytmva::tmvavar("Dtrk2Eta"      , "Dtrk2Eta"                                             , ""     , "trk_{2} #eta"                    , -2   , 2)   ,
    /*9: */ mytmva::tmvavar("Dtrk1DxySig"   , "Dtrk1DxySig := TMath::Abs(Dtrk1Dxy1/Dtrk1DxyError1)"  , ""     , "trk_{1} |D_{xy}/#sigma(D_{xy})|" , 0    , 4)   ,
    /*10:*/ mytmva::tmvavar("Dtrk2DxySig"   , "Dtrk2DxySig := TMath::Abs(Dtrk2Dxy1/Dtrk2DxyError1)"  , ""     , "trk_{2} |D_{xy}/#sigma(D_{xy})|" , 0    , 4)   ,
    /*11:*/ mytmva::tmvavar("Dy"            , "Dy"                                                   , ""     , "y"                               , -2.4 , 2.4) ,
  };
  const mytmva::tmvavar* findvar(std::string varlabel);

  class varval
  {
  public:
    varval(phoD::dtree* nt) : fnt(nt), fvalid(true), rr(new TRandom()) { fval.clear(); fvalid = checkvarlist(); }
    varval(TTree* nttree, bool ishi) : fnt(0), fvalid(true), rr(new TRandom()) { fnt = new phoD::dtree(nttree, ishi); fval.clear(); fvalid = checkvarlist(); }
    float getval(std::string varname, int j) { refreshval(j); if(fval.find(varname) == fval.end()) { std::cout<<"==> "<<__FUNCTION__<<": invalid varname key "<<varname<<std::endl; return 0; } ; return fval[varname]; }
    phoD::dtree* getnt() { return fnt; }
    bool isvalid() { return fvalid; }

  private:
    bool fvalid;
    std::map<std::string, float> fval;
    phoD::dtree* fnt; //~
    TRandom* rr;

    void refreshval(int j)
    {
      bool ll = rr->Integer(2) < 0.5;
      fval["Dmass"]       = j<0?0:fnt->val<float>("Dmass", j);
      fval["Dchi2cl"]     = j<0?0:fnt->val<float>("Dchi2cl", j);
      fval["Ddtheta"]      = j<0?0:fnt->val<float>("Ddtheta", j);
      fval["DdthetaBScorr"]      = j<0?0:fnt->val<float>("DdthetaBScorr", j);
      fval["dls2D"]       = j<0?0:(TMath::Abs(fnt->val<float>("DsvpvDistance_2D", j)/fnt->val<float>("DsvpvDisErr_2D", j)));
      fval["Dtrk1Pt"]     = j<0?0:fnt->val<float>("Dtrk1Pt", j);
      fval["Dtrk2Pt"]     = j<0?0:fnt->val<float>("Dtrk2Pt", j);
      fval["Dtrk1Eta"]    = j<0?0:fnt->val<float>("Dtrk1Eta", j);
      fval["Dtrk2Eta"]    = j<0?0:fnt->val<float>("Dtrk2Eta", j);
      fval["Dtrk1DxySig"] = j<0?0:(TMath::Abs(fnt->val<float>("Dtrk1Dxy1", j)/fnt->val<float>("Dtrk1DxyError1", j)));
      fval["Dtrk2DxySig"] = j<0?0:(TMath::Abs(fnt->val<float>("Dtrk2Dxy1", j)/fnt->val<float>("Dtrk2DxyError1", j)));
      fval["Dy"]          = j<0?0:fnt->val<float>("Dy", j);
    }
    bool checkvarlist() 
    {  
      refreshval(-1);
      for(auto& vn : varlist) 
        { if(fval.find(vn.varname) == fval.end()) { std::cout<<"==> "<<__FUNCTION__<<": invalid varname key "<<vn.varname<<std::endl; return false; } }
      return true;
    }
  };

  std::vector<std::string> argmethods; std::vector<int> argstages;
  std::string mkname(std::string outputname, float ptmin, float ptmax, std::string mymethod, std::string stage,
                     std::vector<std::string> &methods = argmethods, std::vector<int> &stages = argstages);
}

const mytmva::tmvavar* mytmva::findvar(std::string varlabel)
{
  for(auto& vv : varlist)
    {
      if(vv.varname == varlabel) return &vv;
    }
  return 0;
}

std::string mytmva::mkname(std::string outputname, float ptmin, float ptmax, std::string mymethod, std::string stage, 
                           std::vector<std::string> &methods, std::vector<int> &stages)
{
  mymethod = xjjc::str_replaceall(mymethod, " ", "");
  stage = xjjc::str_replaceall(stage, " ", "");
  methods = xjjc::str_divide(mymethod, ",");
  for(auto& ss : xjjc::str_divide(stage, ",")) { stages.push_back(atoi(ss.c_str())); }
  std::string outfname(Form("%s_%s_%s_%s_%s.root", outputname.c_str(),xjjc::str_replaceallspecial(mymethod).c_str(),
                            xjjc::number_to_string(ptmin).c_str(), (ptmax<0?"inf":xjjc::number_to_string(ptmax).c_str()),
                            xjjc::str_replaceall(stage, ",", "-").c_str()));
  return outfname;
}


#endif
