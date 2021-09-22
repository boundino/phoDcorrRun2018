# TMVA training

## Train

### Train on specific variables and get ROC/eff/variable figures
```
# ./run_TMVAClassification.sh [train] [draw curves] [draw curve vs. var] [create BDT tree]
./run_TMVAClassification.sh 1 1
```

### (Optional) Train on different number of variables
- Change `run_TMVAClassification.sh`
```
sequence=1
```
- Run macros (*this might be very slow due to training multiple times*)
```
# ./run_TMVAClassification.sh [train] [draw curves] [draw curve vs. var] [create BDT tree]
./run_TMVAClassification.sh 1 1 1
```

### Check output root files/figures
```
ll rootfiles/ dataset/*/*
```

## Create MVA tree
- Change `run_TMVAClassification.sh`
```
outputmva= # somewhere you can write in
```
The MVA calculation will be applied on `$inputm`
- Run (*this might be slow*)
```
./run_TMVAClassification.sh 0 0 0 1
```

## Note
In `run_TMVAClassification.sh`, 
```
stages="0,10,9,7,6,3,1,2,4,5,8"
```
means the variables that will be used and their order. The definition is as following:

```
####################################################################################################################################
#                                                                                                                                  #
#  0  :  ("Bchi2cl"  , "Bchi2cl"                                                                                        , "FMax")  #
#  1  :  ("dRtrk1"   , "dRtrk1 := TMath::Sqrt(pow(TMath::ACos(TMath::Cos(Bujphi-Btrk1Phi)),2) + pow(Bujeta-Btrk1Eta,2))", "FMin")  #
#  2  :  ("dRtrk2"   , "dRtrk2 := TMath::Sqrt(pow(TMath::ACos(TMath::Cos(Bujphi-Btrk2Phi)),2) + pow(Bujeta-Btrk2Eta,2))", "FMin")  #
#  3  :  ("Qvalue"   , "Qvalue := (Bmass-3.096916-Btktkmass)"                                                           , "FMin")  #
#  4  :  ("Balpha"   , "Balpha"                                                                                         , "FMin")  #
#  5  :  ("costheta" , "costheta := TMath::Cos(Bdtheta)"                                                                , "FMax")  #
#  6  :  ("dls3D"    , "dls3D := TMath::Abs(BsvpvDistance/BsvpvDisErr)"                                                 , "FMax")  #
#  7  :  ("dls2D"    , "dls2D := TMath::Abs(BsvpvDistance_2D/BsvpvDisErr_2D)"                                           , "FMax")  #
#  8  :  ("Btrk1pt"  , "Btrk1Pt"                                                                                        , "FMax")  #
#  9  :  ("Btrk2pt"  , "Btrk2Pt"                                                                                        , "FMax")  #
#  10 :  ("trkptimba", "trkptimba := TMath::Abs((Btrk1Pt-Btrk2Pt) / (Btrk1Pt+Btrk2Pt))"                                 , "FMax")  #
#  11 :  ("By"       , "By"                                                                                             , ""    )  #
#  12 :  ("Bmass"    , "Bmass"                                                                                          , ""    )  #
#  13 :  ("Btrk1Eta" , "Btrk1Eta"                                                                                       , ""    )  #
#  14 :  ("Btrk2Eta" , "Btrk2Eta"                                                                                       , ""    )  #
#                                                                                                                                  #
####################################################################################################################################
```
