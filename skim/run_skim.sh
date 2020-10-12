#!/bin/bash

input=HiForestAOD_PbPbphoMC.root
output=skim_$input

ishi=1
evtfilt=0
# mvafilt: [1] mva [2] cutbase [3] gen-matched
mvafilt=3
hltfilt=0

g++ skim.cc $(root-config --libs --cflags) -I"../includes/" -lTMVA -lXMLIO -g -o skim.exe || exit 1

[[ ${1:-0} -eq 1 ]] && ./skim.exe $input $output $ishi $evtfilt $mvafilt $hltfilt

[[ -f skim.exe ]] && rm skim.exe
