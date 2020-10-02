#!/bin/bash

input=HiForestAOD_101.root
output=skim_HiForestAOD_101.root

ishi=0
evtfilt=0
mvafilt=1
hltfilt=1

g++ skim.cc $(root-config --libs --cflags) -I"../includes/" -lTMVA -lXMLIO -g -o skim.exe || exit 1

[[ ${1:-0} -eq 1 ]] && ./skim.exe $input $output $ishi $evtfilt $mvafilt $hltfilt

[[ -f skim.exe ]] && rm skim.exe
