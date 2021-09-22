#!/bin/bash

inputname=/export/d00/scratch/jwang/BntupleRun2018/crab_Bfinder_samesign_20190513_HIDoubleMuonPsi_HIRun2018A_04Apr2019_v1_1033p1_GoldenJSON_327123_327564_skimhltBsize_ntmix_Xpt10.root
outputname=/export/d00/scratch/jwang/BntupleRun2018/output_mvarandom_samesign
weightdir=noBtrkLH/dataset/weights/rootfiles_TMVA_trainX_sideband_tktk0p2_BDT_BDTG_CutsGA_CutsSA_LD_10p0_inf_0-10-1-2-9_root

g++ mvaprodsimple.C $(root-config --libs --cflags) -g -o mvaprodsimple.exe -lstdc++fs -lTMVA -lXMLIO || exit 1

[[ ${1:-0} -eq 1 ]] && ./mvaprodsimple.exe $inputname "Bfinder/ntmix" $outputname $weightdir

rm mvaprodsimple.exe 