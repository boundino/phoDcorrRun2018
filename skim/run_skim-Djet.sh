#!/bin/bash

# input=rootfiles/HiForestAOD_PuAK4CaloJet80100Eta5p1_100.root ; ishi=1 ;
# input=rootfiles/HiForestAOD_HIAK4CaloJet80_10.root ; ishi=0 ;
# input=rootfiles/HiForestAOD_ppDjetMC_16.root ; ishi=0 ;
input=rootfiles/HiForestAOD_1.root ; ishi=0

evtfilt=0
hltfilt=0
# mvafilt: [1] mva [2] cutbase [3] gen-matched
mvafilt=0
jetptcut=70
removeevent=0

output=rootfiles/skim_${input##*/}

g++ skim-Djet.cc $(root-config --libs --cflags) -I"../includes/" -lTMVA -lXMLIO -g -o skim-Djet.exe || exit 1

echo $input
echo $output

[[ ${1:-0} -eq 1 ]] && ./skim-Djet.exe $input $output $ishi $evtfilt $mvafilt $hltfilt $jetptcut $removeevent

[[ -f skim-Djet.exe ]] && rm skim-Djet.exe
