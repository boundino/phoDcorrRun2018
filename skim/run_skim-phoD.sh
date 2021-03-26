#!/bin/bash

# input=HiForestAOD_PbPbphoMC.root ; ishi=1 ;
input=HiForestAOD_ppphoData.root ; ishi=0 ;
output=skim_$input

evtfilt=0
# mvafilt: [1] mva [2] cutbase [3] gen-matched
mvafilt=3
hltfilt=0
phoEtcut=35
removeevent=0

g++ skim-phoD.cc $(root-config --libs --cflags) -I"../includes/" -lTMVA -lXMLIO -g -o skim-phoD.exe || exit 1

[[ ${1:-0} -eq 1 ]] && ./skim-phoD.exe $input $output $ishi $evtfilt $mvafilt $hltfilt $phoEtcut $removeevent

[[ -f skim-phoD.exe ]] && rm skim-phoD.exe
