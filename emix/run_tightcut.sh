#!/bin/bash

input_trig=/export/d00/scratch/jwang/DntupleRun2018/phodmva_20210217_Dpho_20200921_HIHardProbes_04Apr2019_HIGEDPhoton40_trk1Dpt2_phoEt35_hlt40.root
input_mb=/export/d00/scratch/jwang/DntupleRun2018/phodmva_20210217_Dpho_20210120_HIMinimumBias0_04Apr2019_trk1Dpt2_part_326381-326856_phoEt35_f9568.root
# input_mb=/export/d00/scratch/jwang/DntupleRun2018/phodmva_20210217_Dpho_20210120_HIMinimumBias0_04Apr2019_trk1Dpt2_part_326859-326897_phoEt35_f1452.root
nmix=40
output=${input_trig%%.root}_NMIX${nmix}_photight.root

echo $input_trig
echo $input_mb
echo $output

g++ mixevents_tightcut.cc $(root-config --cflags --libs) -I"../includes/" -g -o mixevents_tightcut.exe || exit 1

RUN_MIX=${1:-0}

[[ $RUN_MIX -eq 1 ]] && ./mixevents_tightcut.exe $input_trig $input_mb $output $nmix

rm mixevents_tightcut.exe

# input_trig=/raid5/data/wangj/DntupleRun2018/phodmva_20200924_Dpho_20200921_HIHardProbes_04Apr2019_HIGEDPhoton40_trk1Dpt2.root
# input_mb=/raid5/data/wangj/DntupleRun2018/phodmva_20210217_Dpho_20210120_HIMinimumBias0_04Apr2019_trk1Dpt2_part_326859-326897_phoEt35_f1452.root
