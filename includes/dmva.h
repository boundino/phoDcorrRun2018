#ifndef __PHOD_DMVA__
#define __PHOD_DMVA__

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

// #include <experimental/filesystem>

#include <TFile.h>
#include <TTree.h>
// #include <TSystem.h>
#include "TMVA/Reader.h"
#include "TMVA/Tools.h"

#include "xjjcuti.h"
#include "xjjrootuti.h"
#include "dtree.h"

namespace phoD
{
  std::vector<float> centbins = {0, 30, 50, 100};
  int ncent = centbins.size() - 1;

  class dmva
  {
  public:
    dmva(std::string weightdir = ".");
    float eval(dtree* dtr, int j, int hiBin);
    bool pass(float mva, float pt, int hiBin);
  private:
    std::vector<std::string> xmlnames_;
    std::vector<TMVA::Reader*> readers_;
    std::vector<TF1*> fmva_;
    std::vector<std::map<std::string, float>> var_;
  };
}

phoD::dmva::dmva(std::string weightdir)
{
  xmlnames_.resize(ncent);
  xmlnames_[0] = weightdir+"/TMVAClassification_BDT_D0pT1p0_NewD0embeddedMCsample_cent0to30.weights.xml";
  xmlnames_[1] = weightdir+"/TMVAClassification_BDT_D0pT1p0_NewD0embeddedMCsample_cent30to50.weights.xml";
  xmlnames_[2] = weightdir+"/TMVAClassification_BDT_D0pT1p0_NewD0embeddedMCsample_cent50to80.weights.xml";
  readers_.resize(ncent);
  var_.resize(ncent);

  for(int i=0; i<ncent; i++)
    {
      readers_[i] = new TMVA::Reader( "!Color:!Silent" );

      // variable
      void *doc = TMVA::gTools().xmlengine().ParseFile(xmlnames_[i].c_str(), TMVA::gTools().xmlenginebuffersize());
      void* rootnode = TMVA::gTools().xmlengine().DocGetRootElement(doc); //node "MethodSetup"
      void* variables = TMVA::gTools().GetChild(rootnode, "Variables");
      UInt_t NVar=0;
      TMVA::gTools().ReadAttr(variables, "NVar", NVar);
      void* var = TMVA::gTools().GetChild(variables, "Variable");
      for(unsigned int k=0;k<NVar;k++)
        {
          std::string varlabel("");
          TMVA::gTools().ReadAttr(var, "Label", varlabel);
          // var_[i][varlabel] = new float;
          readers_[i]->AddVariable(varlabel.c_str(), &(var_[i][varlabel]));
          var = TMVA::gTools().GetNextChild(var);
        }
      // method
      readers_[i]->BookMVA("BDT method", xmlnames_[i].c_str() );
    }

  fmva_.resize(ncent);
  TFile* infcent0to30 = TFile::Open(Form("%s/BDTCuts_finePtBins_PrompD0_PbPb_cent0to30_trkPt1GeV.root", weightdir.c_str()));
  fmva_[0] = xjjroot::copyobject((TF1*)infcent0to30->Get("f_1"), "fcent0to30");
  TFile* infcent30to50 = TFile::Open(Form("%s/BDTCuts_finePtBins_PrompD0_PbPb_cent30to50_trkPt1GeV.root", weightdir.c_str()));
  fmva_[1] = xjjroot::copyobject((TF1*)infcent30to50->Get("f_1"), "fcent30to50");
  TFile* infcent50to80 = TFile::Open(Form("%s/BDTCuts_finePtBins_PrompD0_PbPb_cent50to80_trkPt1GeV.root", weightdir.c_str()));
  fmva_[2] = xjjroot::copyobject((TF1*)infcent50to80->Get("f_1"), "fcent50to80");
}

float phoD::dmva::eval(dtree* dtr, int j, int hiBin)
{
  int icent = xjjc::findibin(centbins, (float)(hiBin/2.));
  var_[icent]["pT"] = dtr->val<float>("Dpt", j);
  var_[icent]["y"] = dtr->val<float>("Dy", j);
  var_[icent]["VtxProb"] = dtr->val<float>("Dchi2cl", j);
  var_[icent]["3DDecayLengthSignificance"] = dtr->val<float>("DsvpvDistance", j) / dtr->val<float>("DsvpvDisErr", j);
  var_[icent]["2DDecayLengthSignificance"] = dtr->val<float>("DsvpvDistance_2D", j) / dtr->val<float>("DsvpvDisErr_2D", j);
  var_[icent]["3DDecayLength"] = dtr->val<float>("DsvpvDistance", j);
  var_[icent]["3DPointingAngle"] = dtr->val<float>("Dalpha", j);
  var_[icent]["2DPointingAngle"] = dtr->val<float>("Ddtheta", j);
  var_[icent]["zDCASignificanceDaugther1"] = dtr->val<float>("Dtrk1Dz1", j) / dtr->val<float>("Dtrk1DzError1", j);
  var_[icent]["zDCASignificanceDaugther2"] = dtr->val<float>("Dtrk2Dz1", j) / dtr->val<float>("Dtrk2DzError1", j);
  var_[icent]["xyDCASignificanceDaugther1"] = dtr->val<float>("Dtrk1Dxy1", j) / dtr->val<float>("Dtrk1DxyError1", j);
  var_[icent]["xyDCASignificanceDaugther2"] = dtr->val<float>("Dtrk1Dxy1", j) / dtr->val<float>("Dtrk1DxyError1", j);
  var_[icent]["NHitD1"] = dtr->val<float>("Dtrk1PixelHit", j) + dtr->val<float>("Dtrk1StripHit", j);
  var_[icent]["NHitD2"] = dtr->val<float>("Dtrk2PixelHit", j) + dtr->val<float>("Dtrk2StripHit", j);
  var_[icent]["pTD1"] = dtr->val<float>("Dtrk1Pt", j);
  var_[icent]["pTD2"] = dtr->val<float>("Dtrk2Pt", j);
  var_[icent]["EtaD1"] = dtr->val<float>("Dtrk1Eta", j);
  var_[icent]["EtaD2"] = dtr->val<float>("Dtrk2Eta", j);
  var_[icent]["pTerrD1"] = dtr->val<float>("Dtrk1PtErr", j);
  var_[icent]["pTerrD2"] = dtr->val<float>("Dtrk2PtErr", j);

  float mva = readers_[icent]->EvaluateMVA("BDT method");

  return mva;
}

bool phoD::dmva::pass(float mva, float pt, int hiBin)
{
  int icent = xjjc::findibin(centbins, (float)(hiBin/2.));
  float mvathre = fmva_[icent]->Eval(pt);
  if(pt > 8) mvathre = fmva_[icent]->Eval(8.);
  return mva > mvathre;
}

#endif
