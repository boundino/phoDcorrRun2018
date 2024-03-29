#include <cstdlib>
#include <iostream>
#include <map>
#include <string>

#include "TChain.h"
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TObjString.h"
#include "TSystem.h"
#include "TROOT.h"

#include "TMVA/Factory.h"
#include "TMVA/DataLoader.h"
#include "TMVA/Tools.h"
#include "TMVA/TMVAGui.h"
#include "TMVA/Config.h"

#include "xjjcuti.h"
#include "xjjrootuti.h"
#include "TMVAClassification.h"
#include "dtree.h"

int TMVAClassification(std::string inputSname, std::string inputBname, std::string mycuts, std::string mycutb, std::string myweights,
                       std::string outputname, float ptmin, float ptmax, float absymin, float absymax, 
                       std::string mymethod, std::string stage)
{
  std::vector<std::string> methods;
  std::vector<int> stages;
  std::string outfname = mytmva::mkname(outputname, ptmin, ptmax, absymin, absymax, mymethod, stage, methods, stages);
  std::string outputstr = xjjc::str_replaceallspecial(outfname);
  if(ptmax < 0) { ptmax = 1.e+10; }

  // The explicit loading of the shared libTMVA is done in TMVAlogon.C, defined in .rootrc
  // if you use your private .rootrc, or run from a different directory, please copy the
  // corresponding lines from .rootrc

  // Methods to be processed can be given as an argument; use format:
  //
  //     mylinux~> root -l TMVAClassification.C\(\"myMethod1,myMethod2,myMethod3\"\)

  //---------------------------------------------------------------
  // This loads the library
  TMVA::Tools::Instance();

  // Default MVA methods to be trained + tested
  std::map<std::string,int> Use;

  // Cut optimisation
  Use["Cuts"]            = 0;
  Use["CutsD"]           = 0;
  Use["CutsPCA"]         = 0;
  Use["CutsGA"]          = 0;
  Use["CutsSA"]          = 0;
  //
  // 1-dimensional likelihood ("naive Bayes estimator")
  Use["Likelihood"]      = 0;
  Use["LikelihoodD"]     = 0; // the "D" extension indicates decorrelated input variables (see option strings)
  Use["LikelihoodPCA"]   = 0; // the "PCA" extension indicates PCA-transformed input variables (see option strings)
  Use["LikelihoodKDE"]   = 0;
  Use["LikelihoodMIX"]   = 0;
  //
  // Mutidimensional likelihood and Nearest-Neighbour methods
  Use["PDERS"]           = 0;
  Use["PDERSD"]          = 0;
  Use["PDERSPCA"]        = 0;
  Use["PDEFoam"]         = 0;
  Use["PDEFoamBoost"]    = 0; // uses generalised MVA method boosting
  Use["KNN"]             = 0; // k-nearest neighbour method
  //
  // Linear Discriminant Analysis
  Use["LD"]              = 0; // Linear Discriminant identical to Fisher
  Use["Fisher"]          = 0;
  Use["FisherG"]         = 0;
  Use["BoostedFisher"]   = 0; // uses generalised MVA method boosting
  Use["HMatrix"]         = 0;
  //
  // Function Discriminant analysis
  Use["FDA_GA"]          = 0; // minimisation of user-defined function using Genetics Algorithm
  Use["FDA_SA"]          = 0;
  Use["FDA_MC"]          = 0;
  Use["FDA_MT"]          = 0;
  Use["FDA_GAMT"]        = 0;
  Use["FDA_MCMT"]        = 0;
  //
  // Neural Networks (all are feed-forward Multilayer Perceptrons)
  Use["MLP"]             = 0; // Recommended ANN
  Use["MLP2"]            = 0; // Recommended ANN + 2 layer
  Use["MLPR"]            = 0; // Recommended ANN + regulator
  Use["MLPBFGS"]         = 0; // Recommended ANN with optional training method
  Use["MLPBNN"]          = 0; // Recommended ANN with BFGS training method and bayesian regulator
  Use["MLPBNN2"]         = 0; // Recommended ANN with BFGS training method and bayesian regulator + 2 layer
  Use["CFMlpANN"]        = 0; // Depreciated ANN from ALEPH
  Use["TMlpANN"]         = 0; // ROOT's own ANN
  Use["DNN"]             = 0;     // Deep Neural Network
  Use["DNN2"]            = 0;     // Deep Neural Network + 4 layer
  Use["DNN_GPU"]         = 0; // CUDA-accelerated DNN training.
  Use["DNN_CPU"]         = 0; // Multi-core accelerated DNN.
  //
  // Support Vector Machine
  Use["SVM"]             = 0;
  //
  // Boosted Decision Trees
  Use["BDT"]             = 0; // uses Adaptive Boost
  Use["BDTG"]            = 0; // uses Gradient Boost
  Use["BDTB"]            = 0; // uses Bagging
  Use["BDTD"]            = 0; // decorrelation + Adaptive Boost
  Use["BDTF"]            = 0; // allow usage of fisher discriminant for node splitting
  //
  // Friedman's RuleFit method, ie, an optimised series of cuts ("rules")
  Use["RuleFit"]         = 0;
  // ---------------------------------------------------------------

  std::cout << std::endl;
  std::cout << "==> Start TMVAClassification" << std::endl;

  // ------>>
  if(mymethod != "")
    {
      for(std::map<std::string,int>::iterator it = Use.begin(); it != Use.end(); it++) it->second = 0;
      for(auto& m : methods)
        {
          if(Use.find(m) != Use.end())
            { Use[m] = 1; std::cout <<"==> " << __FUNCTION__ << ": Registered method " << m << std::endl; }
          else
            { std::cout << "==> Abort " << __FUNCTION__ << ": error: unknown method " << m << "." << std::endl; continue; }
        }
    }
  // <<------

  // Select methods (don't look at this code - not of interest)
  // if (myMethodList != "") {
  //   for (std::map<std::string,int>::iterator it = Use.begin(); it != Use.end(); it++) it->second = 0;

  //   // std::vector<TString> mlist = TMVA::gTools().SplitString( myMethodList, ',' );
  //   for (UInt_t i=0; i<mlist.size(); i++) {
  //     std::string regMethod(mlist[i]);

  //     if (Use.find(regMethod) == Use.end()) {
  //       std::cout << "Method \"" << regMethod << "\" not known in TMVA under this name. Choose among the following:" << std::endl;
  //       for (std::map<std::string,int>::iterator it = Use.begin(); it != Use.end(); it++) std::cout << it->first << " ";
  //       std::cout << std::endl;
  //       return 1;
  //     }
  //     Use[regMethod] = 1;
  //   }
  // }

  // --------------------------------------------------------------------------------------------------

  // Here the preparation phase begins

  // Read training and test data
  // (it is also possible to use ASCII format as input -> see TMVA Users Guide)
  //// TString fname = "./tmva_class_example.root";

  //// if (gSystem->AccessPathName( fname ))  // file does not exist in local directory
  ////    gSystem->Exec("curl -O http://root.cern.ch/files/tmva_class_example.root");

  //// TFile *input = TFile::Open( fname );

  TFile* inputS = TFile::Open(inputSname.c_str());
  TFile* inputB = TFile::Open(inputBname.c_str());

  //// std::cout << "--- TMVAClassification       : Using input file: " << input->GetName() << std::endl;
  std::cout << "--- TMVAClassification       : Using input file: " << inputS->GetName() << " & "<< inputB->GetName() <<std::endl;

  // Register the training and test trees

  //// TTree* signalTree     = (TTree*)input->Get("TreeS");
  //// TTree* background     = (TTree*)input->Get("TreeB");

  TTree* background = (TTree*)inputB->Get("Dfinder/ntDkpi");
  background->AddFriend("hltanalysis/HltTree");
  background->AddFriend("hiEvtAnalyzer/HiTree");
  background->AddFriend("skimanalysis/HltTree");

  TTree* signal = (TTree*)inputS->Get("Dfinder/ntDkpi");
  signal->AddFriend("hltanalysis/HltTree");
  signal->AddFriend("hiEvtAnalyzer/HiTree");
  signal->AddFriend("skimanalysis/HltTree");
 
  // Create a ROOT output file where TMVA will store ntuples, histograms, etc.
  //// TString outfileName( "TMVA.root" );
  //// TFile* outputFile = TFile::Open( outfileName, "RECREATE" );
  xjjroot::mkdir(outfname.c_str());
  TFile* outf = TFile::Open(outfname.c_str(), "RECREATE");

  // Create the factory object. Later you can choose the methods
  // whose performance you'd like to investigate. The factory is
  // the only TMVA object you have to interact with
  //
  // The first argument is the base of the name of all the
  // weightfiles in the directory weight/
  //
  // The second argument is the output file for the training results
  // All TMVA output can be suppressed by removing the "!" (not) in
  // front of the "Silent" argument in the option string
  TMVA::Factory *factory = new TMVA::Factory( "TMVAClassification", outf,
                                              "!V:!Silent:Color:DrawProgressBar:Transformations=I;D;P;G,D:AnalysisType=Classification" );

  TMVA::DataLoader *dataloader = new TMVA::DataLoader("dataset");
  // If you wish to modify default settings
  // (please check "src/Config.h" to see all available global options)
  //
  //    (TMVA::gConfig().GetVariablePlotting()).fTimesRMS = 8.0;
  gSystem->Exec(Form("mkdir -p dataset/weights/%s", outputstr.c_str()));
  (TMVA::gConfig().GetIONames()).fWeightFileDir = Form("weights/%s", outputstr.c_str());

  // Define the input variables that shall be used for the MVA training
  // note that you may also use variable expressions, such as: "3*var1/var2*abs(var3)"
  // [all types of expressions that can also be parsed by TTree::Draw( "expression" )]
  //// dataloader->AddVariable( "myvar1 := var1+var2", 'F' );
  //// dataloader->AddVariable( "myvar2 := var1-var2", "Expression 2", "", 'F' );
  //// dataloader->AddVariable( "var3",                "Variable 3", "units", 'F' );
  //// dataloader->AddVariable( "var4",                "Variable 4", "units", 'F' );

  // Variable
  std::string varinfo = "";
  TString VarSet = "";
  int nvar = 0;
  for(auto& s : stages)
    {
      dataloader->AddVariable(mytmva::varlist[s].var);
      if(mytmva::varlist[s].cutsign != "")
        { VarSet += (Form(":VarProp[%d]=", nvar)+mytmva::varlist[s].cutsign); }
      varinfo += (";"+mytmva::varlist[s].varname);
      std::cout << "==> " << __FUNCTION__ << ": Registered variable " << mytmva::varlist[s].var << std::endl;
      nvar++;
    }
  if(!nvar) { std::cout << "==> Abort " << __FUNCTION__ << ": no variable registered." << std::endl; return 2; }
  std::cout << "==> " << __FUNCTION__ << ": VarSet = " << VarSet << std::endl;

  // Spectator
  // dataloader->AddSpectator("Dmass");

  // You can add so-called "Spectator variables", which are not used in the MVA training,
  // but will appear in the final "TestTree" produced by TMVA. This TestTree will contain the
  // input variables, the response values of all trained MVAs, and the spectator variables

  //// dataloader->AddSpectator( "spec1 := var1*2",  "Spectator 1", "units", 'F' );
  //// dataloader->AddSpectator( "spec2 := var1*3",  "Spectator 2", "units", 'F' );

  // global event weights per tree (see below for setting event-wise weights)
  Double_t signalWeight     = 1.0;
  Double_t backgroundWeight = 1.0;

  // You can add an arbitrary number of signal or background trees
  //// dataloader->AddSignalTree    ( signalTree,     signalWeight );
  //// dataloader->AddBackgroundTree( background, backgroundWeight );
  dataloader->AddSignalTree    ( signal,     signalWeight );
  dataloader->AddBackgroundTree( background, backgroundWeight );

  // To give different trees for training and testing, do as follows:
  //
  //     dataloader->AddSignalTree( signalTrainingTree, signalTrainWeight, "Training" );
  //     dataloader->AddSignalTree( signalTestTree,     signalTestWeight,  "Test" );

  // Use the following code instead of the above two or four lines to add signal and background
  // training and test events "by hand"
  // NOTE that in this case one should not give expressions (such as "var1+var2") in the input
  //      variable definition, but simply compute the expression before adding the event
  // ```cpp
  // // --- begin ----------------------------------------------------------
  // std::vector<Double_t> vars( 4 ); // vector has size of number of input variables
  // Float_t  treevars[4], weight;
  //
  // // Signal
  // for (UInt_t ivar=0; ivar<4; ivar++) signalTree->SetBranchAddress( Form( "var%i", ivar+1 ), &(treevars[ivar]) );
  // for (UInt_t i=0; i<signalTree->GetEntries(); i++) {
  //    signalTree->GetEntry(i);
  //    for (UInt_t ivar=0; ivar<4; ivar++) vars[ivar] = treevars[ivar];
  //    // add training and test events; here: first half is training, second is testing
  //    // note that the weight can also be event-wise
  //    if (i < signalTree->GetEntries()/2.0) dataloader->AddSignalTrainingEvent( vars, signalWeight );
  //    else                              dataloader->AddSignalTestEvent    ( vars, signalWeight );
  // }
  //
  // // Background (has event weights)
  // background->SetBranchAddress( "weight", &weight );
  // for (UInt_t ivar=0; ivar<4; ivar++) background->SetBranchAddress( Form( "var%i", ivar+1 ), &(treevars[ivar]) );
  // for (UInt_t i=0; i<background->GetEntries(); i++) {
  //    background->GetEntry(i);
  //    for (UInt_t ivar=0; ivar<4; ivar++) vars[ivar] = treevars[ivar];
  //    // add training and test events; here: first half is training, second is testing
  //    // note that the weight can also be event-wise
  //    if (i < background->GetEntries()/2) dataloader->AddBackgroundTrainingEvent( vars, backgroundWeight*weight );
  //    else                                dataloader->AddBackgroundTestEvent    ( vars, backgroundWeight*weight );
  // }
  // // --- end ------------------------------------------------------------
  // ```
  // End of tree registration

  // Set individual event weights (the variables must exist in the original TTree)
  // -  for signal    : `dataloader->SetSignalWeightExpression    ("weight1*weight2");`
  // -  for background: `dataloader->SetBackgroundWeightExpression("weight1*weight2");`
  //// dataloader->SetBackgroundWeightExpression( "weight" );

  dataloader->SetSignalWeightExpression(myweights.c_str());

  // Apply additional cuts on the signal and background samples (can be different)
  //// TCut mycuts = ""; // for example: TCut mycuts = "abs(var1)<0.5 && abs(var2-0.5)<1";
  //// TCut mycutb = ""; // for example: TCut mycutb = "abs(var1)<0.5";

  TString cuts = Form("(%s) && Dpt>%f && Dpt<%f && TMath::Abs(Dy)>=%f && TMath::Abs(Dy)<%f", mycuts.c_str(), ptmin, ptmax, absymin, absymax);
  TString cutb = Form("(%s) && Dpt>%f && Dpt<%f && TMath::Abs(Dy)>=%f && TMath::Abs(Dy)<%f", mycutb.c_str(), ptmin, ptmax, absymin, absymax);

  TCut mycutS = (TCut)cuts;
  TCut mycutB = (TCut)cutb;

  // Tell the dataloader how to use the training and testing events
  //
  // If no numbers of events are given, half of the events in the tree are used
  // for training, and the other half for testing:
  //
  //    dataloader->PrepareTrainingAndTestTree( mycut, "SplitMode=random:!V" );
  //
  // To also specify the number of testing events, use:
  //
  //    dataloader->PrepareTrainingAndTestTree( mycut,
  //         "NSigTrain=3000:NBkgTrain=3000:NSigTest=3000:NBkgTest=3000:SplitMode=Random:!V" );
  //// dataloader->PrepareTrainingAndTestTree( mycuts, mycutb,
  ////                                      "nTrain_Signal=1000:nTrain_Background=1000:SplitMode=Random:NormMode=NumEvents:!V" );
  dataloader->PrepareTrainingAndTestTree( mycutS, mycutB,
                                          // "nTrain_Signal=10000:nTrain_Background=10000:nTest_Signal=10000:nTest_Background=10000:SplitMode=Random:NormMode=NumEvents:!V" );
                                          // "nTrain_Signal=0:nTrain_Background=100000:nTest_Signal=0:nTest_Background=100000:SplitMode=Random:NormMode=NumEvents:!V" );
                                          "nTrain_Signal=0:nTrain_Background=0:nTest_Signal=0:nTest_Background=0:SplitMode=Random:NormMode=NumEvents:!V" );

  // ### Book MVA methods
  //
  // Please lookup the various method configuration options in the corresponding cxx files, eg:
  // src/MethoCuts.cxx, etc, or here: http://tmva.sourceforge.net/optionRef.html
  // it is possible to preset ranges in the option string in which the cut optimisation should be done:
  // "...:CutRangeMin[2]=-1:CutRangeMax[2]=1"...", where [2] is the third input variable

  // Cut optimisation
  if (Use["Cuts"])
    factory->BookMethod( dataloader, TMVA::Types::kCuts, "Cuts",
                         "!H:!V:FitMethod=MC:EffSel:SampleSize=200000:VarProp=FSmart" );

  TString CutsDExp = "!H:!V:FitMethod=MC:EffSel:SampleSize=400000:VarProp=FSmart:VarTransform=Decorrelate";
  CutsDExp+=VarSet;
  if (Use["CutsD"])
    factory->BookMethod( dataloader, TMVA::Types::kCuts, "CutsD",
                         //// "!H:!V:FitMethod=MC:EffSel:SampleSize=200000:VarProp=FSmart:VarTransform=Decorrelate" );
                         CutsDExp.Data());

  TString CutsPCAExp = "!H:!V:FitMethod=MC:EffSel:SampleSize=400000:VarProp=FSmart:VarTransform=PCA";
  CutsPCAExp+=VarSet;
  if (Use["CutsPCA"])
    factory->BookMethod( dataloader, TMVA::Types::kCuts, "CutsPCA",
                         //// "!H:!V:FitMethod=MC:EffSel:SampleSize=200000:VarProp=FSmart:VarTransform=PCA" );
                         CutsPCAExp.Data());

  TString CutsGAExp = "H:!V:FitMethod=GA:EffSel:Steps=40:Cycles=3:PopSize=200:SC_steps=10:SC_rate=5:SC_factor=0.95";
  CutsGAExp+=VarSet;
  if (Use["CutsGA"])
    factory->BookMethod( dataloader, TMVA::Types::kCuts, "CutsGA",
                         //// "H:!V:FitMethod=GA:CutRangeMin[0]=-10:CutRangeMax[0]=10:VarProp[1]=FMax:EffSel:Steps=30:Cycles=3:PopSize=400:SC_steps=10:SC_rate=5:SC_factor=0.95" );
                         CutsGAExp.Data());

  TString CutsSAExp = "!H:!V:FitMethod=SA:EffSel:MaxCalls=15000:KernelTemp=IncAdaptive:InitialTemp=1e+6:MinTemp=1e-6:Eps=1e-10:UseDefaultScale";
  CutsSAExp+=VarSet;
  if (Use["CutsSA"])
    factory->BookMethod( dataloader, TMVA::Types::kCuts, "CutsSA",
                         //// "!H:!V:FitMethod=SA:EffSel:MaxCalls=150000:KernelTemp=IncAdaptive:InitialTemp=1e+6:MinTemp=1e-6:Eps=1e-10:UseDefaultScale" );
                         CutsSAExp.Data());

  // Likelihood ("naive Bayes estimator")
  if (Use["Likelihood"])
    factory->BookMethod( dataloader, TMVA::Types::kLikelihood, "Likelihood",
                         "H:!V:TransformOutput:PDFInterpol=Spline2:NSmoothSig[0]=20:NSmoothBkg[0]=20:NSmoothBkg[1]=10:NSmooth=1:NAvEvtPerBin=50" );

  // Decorrelated likelihood
  if (Use["LikelihoodD"])
    factory->BookMethod( dataloader, TMVA::Types::kLikelihood, "LikelihoodD",
                         "!H:!V:TransformOutput:PDFInterpol=Spline2:NSmoothSig[0]=20:NSmoothBkg[0]=20:NSmooth=5:NAvEvtPerBin=50:VarTransform=Decorrelate" );

  // PCA-transformed likelihood
  if (Use["LikelihoodPCA"])
    factory->BookMethod( dataloader, TMVA::Types::kLikelihood, "LikelihoodPCA",
                         "!H:!V:!TransformOutput:PDFInterpol=Spline2:NSmoothSig[0]=20:NSmoothBkg[0]=20:NSmooth=5:NAvEvtPerBin=50:VarTransform=PCA" );

  // Use a kernel density estimator to approximate the PDFs
  if (Use["LikelihoodKDE"])
    factory->BookMethod( dataloader, TMVA::Types::kLikelihood, "LikelihoodKDE",
                         "!H:!V:!TransformOutput:PDFInterpol=KDE:KDEtype=Gauss:KDEiter=Adaptive:KDEFineFactor=0.3:KDEborder=None:NAvEvtPerBin=50" );

  // Use a variable-dependent mix of splines and kernel density estimator
  if (Use["LikelihoodMIX"])
    factory->BookMethod( dataloader, TMVA::Types::kLikelihood, "LikelihoodMIX",
                         "!H:!V:!TransformOutput:PDFInterpolSig[0]=KDE:PDFInterpolBkg[0]=KDE:PDFInterpolSig[1]=KDE:PDFInterpolBkg[1]=KDE:PDFInterpolSig[2]=Spline2:PDFInterpolBkg[2]=Spline2:PDFInterpolSig[3]=Spline2:PDFInterpolBkg[3]=Spline2:KDEtype=Gauss:KDEiter=Nonadaptive:KDEborder=None:NAvEvtPerBin=50" );

  // Test the multi-dimensional probability density estimator
  // here are the options strings for the MinMax and RMS methods, respectively:
  //
  //      "!H:!V:VolumeRangeMode=MinMax:DeltaFrac=0.2:KernelEstimator=Gauss:GaussSigma=0.3" );
  //      "!H:!V:VolumeRangeMode=RMS:DeltaFrac=3:KernelEstimator=Gauss:GaussSigma=0.3" );
  if (Use["PDERS"])
    factory->BookMethod( dataloader, TMVA::Types::kPDERS, "PDERS",
                         "!H:!V:NormTree=T:VolumeRangeMode=Adaptive:KernelEstimator=Gauss:GaussSigma=0.3:NEventsMin=400:NEventsMax=600" );

  if (Use["PDERSD"])
    factory->BookMethod( dataloader, TMVA::Types::kPDERS, "PDERSD",
                         "!H:!V:VolumeRangeMode=Adaptive:KernelEstimator=Gauss:GaussSigma=0.3:NEventsMin=400:NEventsMax=600:VarTransform=Decorrelate" );

  if (Use["PDERSPCA"])
    factory->BookMethod( dataloader, TMVA::Types::kPDERS, "PDERSPCA",
                         "!H:!V:VolumeRangeMode=Adaptive:KernelEstimator=Gauss:GaussSigma=0.3:NEventsMin=400:NEventsMax=600:VarTransform=PCA" );

  // Multi-dimensional likelihood estimator using self-adapting phase-space binning
  if (Use["PDEFoam"])
    factory->BookMethod( dataloader, TMVA::Types::kPDEFoam, "PDEFoam",
                         "!H:!V:SigBgSeparate=F:TailCut=0.001:VolFrac=0.0666:nActiveCells=500:nSampl=2000:nBin=5:Nmin=100:Kernel=None:Compress=T" );

  if (Use["PDEFoamBoost"])
    factory->BookMethod( dataloader, TMVA::Types::kPDEFoam, "PDEFoamBoost",
                         "!H:!V:Boost_Num=30:Boost_Transform=linear:SigBgSeparate=F:MaxDepth=4:UseYesNoCell=T:DTLogic=MisClassificationError:FillFoamWithOrigWeights=F:TailCut=0:nActiveCells=500:nBin=20:Nmin=400:Kernel=None:Compress=T" );

  // K-Nearest Neighbour classifier (KNN)
  if (Use["KNN"])
    factory->BookMethod( dataloader, TMVA::Types::kKNN, "KNN",
                         "H:nkNN=20:ScaleFrac=0.8:SigmaFact=1.0:Kernel=Gaus:UseKernel=F:UseWeight=T:!Trim" );

  // H-Matrix (chi2-squared) method
  if (Use["HMatrix"])
    factory->BookMethod( dataloader, TMVA::Types::kHMatrix, "HMatrix", "!H:!V:VarTransform=None" );

  // Linear discriminant (same as Fisher discriminant)
  if (Use["LD"])
    factory->BookMethod( dataloader, TMVA::Types::kLD, "LD", "H:!V:VarTransform=None:CreateMVAPdfs:PDFInterpolMVAPdf=Spline2:NbinsMVAPdf=50:NsmoothMVAPdf=10" );

  // Fisher discriminant (same as LD)
  if (Use["Fisher"])
    factory->BookMethod( dataloader, TMVA::Types::kFisher, "Fisher", "H:!V:Fisher:VarTransform=None:CreateMVAPdfs:PDFInterpolMVAPdf=Spline2:NbinsMVAPdf=50:NsmoothMVAPdf=10" );

  // Fisher with Gauss-transformed input variables
  if (Use["FisherG"])
    factory->BookMethod( dataloader, TMVA::Types::kFisher, "FisherG", "H:!V:VarTransform=Gauss" );

  // Composite classifier: ensemble (tree) of boosted Fisher classifiers
  if (Use["BoostedFisher"])
    factory->BookMethod( dataloader, TMVA::Types::kFisher, "BoostedFisher",
                         "H:!V:Boost_Num=20:Boost_Transform=log:Boost_Type=AdaBoost:Boost_AdaBoostBeta=0.2:!Boost_DetailedMonitoring" );

  // Function discrimination analysis (FDA) -- test of various fitters - the recommended one is Minuit (or GA or SA)
  if (Use["FDA_MC"])
    factory->BookMethod( dataloader, TMVA::Types::kFDA, "FDA_MC",
                         "H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x2+(4)*x3:ParRanges=(-1,1);(-10,10);(-10,10);(-10,10);(-10,10):FitMethod=MC:SampleSize=100000:Sigma=0.1" );

  if (Use["FDA_GA"]) // can also use Simulated Annealing (SA) algorithm (see Cuts_SA options])
    factory->BookMethod( dataloader, TMVA::Types::kFDA, "FDA_GA",
                         "H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x2+(4)*x3:ParRanges=(-1,1);(-10,10);(-10,10);(-10,10);(-10,10):FitMethod=GA:PopSize=300:Cycles=3:Steps=20:Trim=True:SaveBestGen=1" );

  if (Use["FDA_SA"]) // can also use Simulated Annealing (SA) algorithm (see Cuts_SA options])
    factory->BookMethod( dataloader, TMVA::Types::kFDA, "FDA_SA",
                         "H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x2+(4)*x3:ParRanges=(-1,1);(-10,10);(-10,10);(-10,10);(-10,10):FitMethod=SA:MaxCalls=15000:KernelTemp=IncAdaptive:InitialTemp=1e+6:MinTemp=1e-6:Eps=1e-10:UseDefaultScale" );

  if (Use["FDA_MT"])
    factory->BookMethod( dataloader, TMVA::Types::kFDA, "FDA_MT",
                         "H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x2+(4)*x3:ParRanges=(-1,1);(-10,10);(-10,10);(-10,10);(-10,10):FitMethod=MINUIT:ErrorLevel=1:PrintLevel=-1:FitStrategy=2:UseImprove:UseMinos:SetBatch" );

  if (Use["FDA_GAMT"])
    factory->BookMethod( dataloader, TMVA::Types::kFDA, "FDA_GAMT",
                         "H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x2+(4)*x3:ParRanges=(-1,1);(-10,10);(-10,10);(-10,10);(-10,10):FitMethod=GA:Converger=MINUIT:ErrorLevel=1:PrintLevel=-1:FitStrategy=0:!UseImprove:!UseMinos:SetBatch:Cycles=1:PopSize=5:Steps=5:Trim" );

  if (Use["FDA_MCMT"])
    factory->BookMethod( dataloader, TMVA::Types::kFDA, "FDA_MCMT",
                         "H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x2+(4)*x3:ParRanges=(-1,1);(-10,10);(-10,10);(-10,10);(-10,10):FitMethod=MC:Converger=MINUIT:ErrorLevel=1:PrintLevel=-1:FitStrategy=0:!UseImprove:!UseMinos:SetBatch:SampleSize=20" );

  // TMVA ANN: MLP (recommended ANN) -- all ANNs in TMVA are Multilayer Perceptrons
  if (Use["MLP"])
    factory->BookMethod( dataloader, TMVA::Types::kMLP, "MLP", "H:!V:NeuronType=tanh:VarTransform=N:NCycles=600:HiddenLayers=N+5:TestRate=5:!UseRegulator" );

  if (Use["MLP2"])
    factory->BookMethod( dataloader, TMVA::Types::kMLP, "MLP2", "H:!V:NeuronType=tanh:VarTransform=N:NCycles=600:HiddenLayers=N+5,N:TestRate=5:!UseRegulator" );

  if (Use["MLPR"])
    factory->BookMethod( dataloader, TMVA::Types::kMLP, "MLPR", "H:!V:NeuronType=tanh:VarTransform=N:NCycles=600:HiddenLayers=N+5:TestRate=5:UseRegulator" );

  if (Use["MLPBFGS"])
    factory->BookMethod( dataloader, TMVA::Types::kMLP, "MLPBFGS", "H:!V:NeuronType=tanh:VarTransform=N:NCycles=600:HiddenLayers=N+5:TestRate=5:TrainingMethod=BFGS:!UseRegulator" );

  if (Use["MLPBNN"])
    factory->BookMethod( dataloader, TMVA::Types::kMLP, "MLPBNN", "H:!V:NeuronType=tanh:VarTransform=N:NCycles=600:HiddenLayers=N+5:TestRate=5:TrainingMethod=BFGS:UseRegulator" ); // BFGS training with bayesian regulators

  if (Use["MLPBNN2"])
    factory->BookMethod( dataloader, TMVA::Types::kMLP, "MLPBNN2", "H:!V:NeuronType=tanh:VarTransform=N:NCycles=600:HiddenLayers=N+5,N:TestRate=5:TrainingMethod=BFGS:UseRegulator" ); // BFGS training with bayesian regulators


  // Multi-architecture DNN implementation.
  //// if (Use["DNN"])
  if (Use["DNN"] || Use["DNN2"] || Use["DNN_GPU"] || Use["DNN_CPU"])
    {
      // General layout.
      TString layoutString ("Layout=TANH|128,TANH|128,TANH|128,LINEAR");
      TString layoutString2 ("Layout=TANH|256,TANH|256,TANH|128,TANH|128,LINEAR");

      // Training strategies.
      TString training0("LearningRate=1e-1,Momentum=0.9,Repetitions=1,"
                        "ConvergenceSteps=20,BatchSize=256,TestRepetitions=10,"
                        "WeightDecay=1e-4,Regularization=L2,"
                        "DropConfig=0.0+0.5+0.5+0.5, Multithreading=True");
      TString training1("LearningRate=1e-2,Momentum=0.9,Repetitions=1,"
                        "ConvergenceSteps=20,BatchSize=256,TestRepetitions=10,"
                        "WeightDecay=1e-4,Regularization=L2,"
                        "DropConfig=0.0+0.0+0.0+0.0, Multithreading=True");
      TString training2("LearningRate=1e-3,Momentum=0.0,Repetitions=1,"
                        "ConvergenceSteps=20,BatchSize=256,TestRepetitions=10,"
                        "WeightDecay=1e-4,Regularization=L2,"
                        "DropConfig=0.0+0.0+0.0+0.0, Multithreading=True");
      TString trainingStrategyString ("TrainingStrategy=");
      trainingStrategyString += training0 + "|" + training1 + "|" + training2;

      // General Options.
      TString dnnOptions ("!H:V:ErrorStrategy=CROSSENTROPY:VarTransform=N:"
                          "WeightInitialization=XAVIERUNIFORM");
      dnnOptions.Append (":"); dnnOptions.Append (layoutString);
      dnnOptions.Append (":"); dnnOptions.Append (trainingStrategyString);

      TString dnnOptions2 ("!H:V:ErrorStrategy=CROSSENTROPY:VarTransform=N:"
                           "WeightInitialization=XAVIERUNIFORM");
      dnnOptions2.Append (":"); dnnOptions2.Append (layoutString2);
      dnnOptions2.Append (":"); dnnOptions2.Append (trainingStrategyString);

      // Standard implementation, no dependencies.
      //// TString stdOptions = dnnOptions + ":Architecture=STANDARD";
      //// factory->BookMethod(dataloader, TMVA::Types::kDNN, "DNN", stdOptions);

      if (Use["DNN"]) {
        TString stdOptions = dnnOptions + ":Architecture=STANDARD";
        factory->BookMethod(dataloader, TMVA::Types::kDNN, "DNN", stdOptions);
      }

      if (Use["DNN2"]) {
        TString stdOptions2 = dnnOptions2 + ":Architecture=STANDARD";
        factory->BookMethod(dataloader, TMVA::Types::kDNN, "DNN2", stdOptions2);
      }

      // Cuda implementation.
      if (Use["DNN_GPU"]) {
        TString gpuOptions = dnnOptions + ":Architecture=GPU";
        factory->BookMethod(dataloader, TMVA::Types::kDNN, "DNN GPU", gpuOptions);
      }
      // Multi-core CPU implementation.
      if (Use["DNN_CPU"]) {
        TString cpuOptions = dnnOptions + ":Architecture=CPU";
        factory->BookMethod(dataloader, TMVA::Types::kDNN, "DNN CPU", cpuOptions);
      }
    }

  // CF(Clermont-Ferrand)ANN
  if (Use["CFMlpANN"])
    factory->BookMethod( dataloader, TMVA::Types::kCFMlpANN, "CFMlpANN", "!H:!V:NCycles=2000:HiddenLayers=N+1,N"  ); // n_cycles:#nodes:#nodes:...

  // Tmlp(Root)ANN
  if (Use["TMlpANN"])
    factory->BookMethod( dataloader, TMVA::Types::kTMlpANN, "TMlpANN", "!H:!V:NCycles=200:HiddenLayers=N+1,N:LearningMethod=BFGS:ValidationFraction=0.3"  ); // n_cycles:#nodes:#nodes:...

  // Support Vector Machine
  if (Use["SVM"])
    factory->BookMethod( dataloader, TMVA::Types::kSVM, "SVM", "Gamma=0.25:Tol=0.001:VarTransform=Norm" );

  // Boosted Decision Trees
  if (Use["BDTG"]) // Gradient Boost
    factory->BookMethod( dataloader, TMVA::Types::kBDT, "BDTG",
                         "!H:!V:NTrees=1000:MinNodeSize=2.5%:BoostType=Grad:Shrinkage=0.10:UseBaggedBoost:BaggedSampleFraction=0.5:nCuts=20:MaxDepth=2" );
  // "!H:!V:NTrees=505:MinNodeSize=15.5:BoostType=Grad:Shrinkage=0.275:UseBaggedBoost:BaggedSampleFraction=0.5:nCuts=20:MaxDepth=3" );

  if (Use["BDT"])  // Adaptive Boost
    factory->BookMethod( dataloader, TMVA::Types::kBDT, "BDT",
                         "!H:!V:NTrees=850:MinNodeSize=2.5%:MaxDepth=3:BoostType=AdaBoost:AdaBoostBeta=0.5:UseBaggedBoost:BaggedSampleFraction=0.5:SeparationType=GiniIndex:nCuts=20" );

  if (Use["BDTB"]) // Bagging
    factory->BookMethod( dataloader, TMVA::Types::kBDT, "BDTB",
                         "!H:!V:NTrees=400:BoostType=Bagging:SeparationType=GiniIndex:nCuts=20" );

  if (Use["BDTD"]) // Decorrelation + Adaptive Boost
    factory->BookMethod( dataloader, TMVA::Types::kBDT, "BDTD",
                         "!H:!V:NTrees=400:MinNodeSize=5%:MaxDepth=3:BoostType=AdaBoost:SeparationType=GiniIndex:nCuts=20:VarTransform=Decorrelate" );

  if (Use["BDTF"])  // Allow Using Fisher discriminant in node splitting for (strong) linearly correlated variables
    factory->BookMethod( dataloader, TMVA::Types::kBDT, "BDTF",
                         "!H:!V:NTrees=50:MinNodeSize=2.5%:UseFisherCuts:MaxDepth=3:BoostType=AdaBoost:AdaBoostBeta=0.5:SeparationType=GiniIndex:nCuts=20" );

  // RuleFit -- TMVA implementation of Friedman's method
  if (Use["RuleFit"])
    factory->BookMethod( dataloader, TMVA::Types::kRuleFit, "RuleFit",
                         "H:!V:RuleFitModule=RFTMVA:Model=ModRuleLinear:MinImp=0.001:RuleMinDist=0.001:NTrees=20:fEventsMin=0.01:fEventsMax=0.5:GDTau=-1.0:GDTauPrec=0.01:GDStep=0.01:GDNSteps=10000:GDErrScale=1.02" );

  // For an example of the category classifier usage, see: TMVAClassificationCategory
  //
  // --------------------------------------------------------------------------------------------------
  //  Now you can optimize the setting (configuration) of the MVAs using the set of training events
  // STILL EXPERIMENTAL and only implemented for BDT's !
  //
  //     factory->OptimizeAllMethods("SigEffAt001","Scan");
  //     factory->OptimizeAllMethods("ROCIntegral","FitGA");
  //
  // --------------------------------------------------------------------------------------------------

  // Now you can tell the factory to train, test, and evaluate the MVAs
  //
  // Train MVAs using the set of training events
  factory->TrainAllMethods();

  // Evaluate all MVAs using the set of test events
  factory->TestAllMethods();

  // Evaluate and compare performance of all configured MVAs
  factory->EvaluateAllMethods();

  // --------------------------------------------------------------

  outf->cd("dataset");
  TTree* info = new TTree("tmvainfo", "TMVA info");
  info->Branch("cuts", &cuts);
  info->Branch("cutb", &cutb);
  info->Branch("var", &varinfo);
  info->Fill();
  info->Write();

  // Save the output
  outf->Close();

  std::cout << "==> Wrote root file: " << outf->GetName() << std::endl;
  std::cout << "==> TMVAClassification is done!" << std::endl;

  delete factory;
  delete dataloader;
  // Launch the GUI for the root macros
  if (!gROOT->IsBatch()) TMVA::TMVAGui( outfname.c_str() );

  // gSystem->Exec(Form("mv dataset/weights/*.* dataset/weights/%s/", outputstr.c_str()));

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc==9)
    { 
      for(int j=0; j<mytmva::nabsybins; j++)
        for(int i=0; i<mytmva::nptbins; i++)
          TMVAClassification(argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], mytmva::ptbins[i], mytmva::ptbins[i+1], mytmva::absybins[j], mytmva::absybins[j+1], argv[7], argv[8]); 
      return 0;
    }

  return 1;
}
