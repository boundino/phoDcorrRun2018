#!/bin/bash

input_trig=/raid5/data/wangj/DntupleRun2018/phodmva_20200924_Dpho_20200921_HIHardProbes_04Apr2019_HIGEDPhoton40_trk1Dpt2.root
input_mb=/raid5/data/wangj/DntupleRun2018/phodmva_20210217_Dpho_20210120_HIMinimumBias0_04Apr2019_trk1Dpt2_part_326859-326897_phoEt35_f1452.root
nmix=2
output=${input_trig%%.root}_NMIX${nmix}.root

echo $input_trig
echo $input_mb
echo $output

g++ mixevents.cc $(root-config --cflags --libs) -I"../includes/" -g -o mixevents.exe || exit 1

RUN_MIX=${1:-0}

[[ $RUN_MIX -eq 1 ]] && ./mixevents.exe $input_trig $input_mb $output $nmix 10000

rm mixevents.exe
