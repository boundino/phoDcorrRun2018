#!/bin/bash

# input=/raid5/data/wangj/DntupleRun2018/phodmva_20200926_Dpho_20200924_QCDPhoton_pThat_30_Filter30GeV_TuneCP5_HydjetDrumMB_trk1Dpt2.root
input=/raid5/data/wangj/DntupleRun2018/phodgmatch_20201011_Dpho_20200924_QCDPhoton_pThat_30_Filter30GeV_TuneCP5_HydjetDrumMB_trk1Dpt2.root

g++ printxs.cc $(root-config --libs --cflags) -I"../includes/" -g -o printxs.exe || exit 1

./printxs.exe $input

rm printxs.exe
