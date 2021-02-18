#!/bin/bash

isyst=(0 1 2)

#
input=(
    "/raid5/data/wangj/DntupleRun2017/phodcut_20210201_Dpho_20201104_Pythia8_DzeroToKPi_prompt_Dpt0p0_Pthat0_TuneCP5_5020GeV_trk1Dpt2_phoEt30.root    eff_pp_prompt    0"
    "/raid5/data/wangj/DntupleRun2017/phodcut_20210201_Dpho_20201104_Pythia8_DzeroToKPi_inclusive_Dpt0p0_Pthat0_TuneCP5_5020GeV_trk1Dpt2_phoEt30.root eff_pp_inclusive 0"
    "/raid5/data/wangj/DntupleRun2018/phodmva_20210201_Dpho_20200924_PrmtD0_TuneCP5_HydjetDrumMB_5p02TeV_pythia8_trk1Dpt2_phoEt30.root                eff_PbPb_prompt  1"
)

##
run_savehist=${1:-0}
run_drawhist=${2:-0}

##
[[ $run_savehist -eq 1 || $# == 0 ]] && { g++ eff_savehist.cc $(root-config --libs --cflags) -I"../includes/" -g -o eff_savehist.exe || exit 1 ; }
[[ $run_drawhist -eq 1 || $# == 0 ]] && { g++ eff_drawhist.cc $(root-config --libs --cflags) -I"../includes/" -g -o eff_drawhist.exe || exit 1 ; }

for i in ${isyst[@]}
do
    [[ $run_savehist -eq 1 ]] && { ./eff_savehist.exe ${input[i]} ; }
    argv=(${input[i]})
    [[ $run_drawhist -eq 1 ]] && { ./eff_drawhist.exe ${argv[1]} ${argv[2]} ; }
done

rm eff_savehist.exe > /dev/null 2>&1
rm eff_drawhist.exe > /dev/null 2>&1

