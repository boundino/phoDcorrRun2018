#!/bin/bash

input=HiForestAOD_1.root
output=skim_HiForestAOD_1.root

g++ skim.cc $(root-config --libs --cflags) -I"../includes/" -lTMVA -lXMLIO -g -o skim.exe || exit 1

[[ ${1:-0} -eq 1 ]] && ./skim.exe $input $output 

[[ -f skim.exe ]] && rm skim.exe
