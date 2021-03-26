#!/bin/bash

input_trig=/export/d00/scratch/jwang/DntupleRun2018/djtmva_20210322_Djet_20210315_HIHardProbes_04Apr2019_PuAK4CaloJet80100Eta5p1_trk1Dpt2_jtpt70.root
input_mb=/export/d00/scratch/jwang/DntupleRun2018/djtmva_20210322_Djet_20210315_HIMinimumBias0_04Apr2019_trk1Dpt2_part_jtpt70_f3786.root

nmix=4
output=${input_trig%%.root}_NMIX${nmix}_jt80.root

echo $input_trig
echo $input_mb
echo $output

g++ mixevents_Djet.cc $(root-config --cflags --libs) -I"../includes/" -g -o mixevents_Djet.exe || exit 1

RUN_MIX=${1:-0}

[[ $RUN_MIX -eq 1 ]] && ./mixevents_Djet.exe $input_trig $input_mb $output $nmix

rm mixevents_Djet.exe

