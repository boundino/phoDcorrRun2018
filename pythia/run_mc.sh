#!/bin/bash

ikine=(1)
jsyst=(1) # ishi

#
config=(
    "2 100 1.2 0 90 40 1.442"  # 0
    "5 100 1.2 0 90 40 1.442"  # 1
)

tag="prel_finebin"

input=(
    /raid5/data/wangj/DntupleRun2018/phodmva_20200926_Dpho_20200924_QCDPhoton_Filter30GeV_TuneCP5_HydjetDrumMB_trk1Dpt2_pthatweight.root
    /raid5/data/wangj/DntupleRun2018/phodgmatch_20201011_Dpho_20200924_QCDPhoton_Filter30GeV_TuneCP5_HydjetDrumMB_trk1Dpt2_pthatweight.root
)
ishi=(0 1)

##
run_savehist=${1:-0}
# run_drawhist=${2:-0}

##
g++ getfname.cc -I"../includes/" $(root-config --libs --cflags) -g -o getfname.exe || { rm *.exe 2> /dev/null ; exit 1 ; }
[[ $run_savehist -eq 1 || $# == 0 ]] && { g++ mcana_savehist.cc $(root-config --libs --cflags) -I"../includes/" -g -o mcana_savehist.exe || exit 1 ; }
# [[ $run_drawhist -eq 1 || $# == 0 ]] && { g++ mcana_drawhist.cc $(root-config --libs --cflags) -I"../includes/" -g -o mcana_drawhist.exe || exit 1 ; }

for i in ${ikine[@]}
do
    for j in ${jsyst[@]}
    do
        tagki=$(./getfname.exe ${ishi[j]} ${config[i]})

        [[ $run_savehist -eq 1 ]] && { ./mcana_savehist.exe ${input[j]} $tag ${ishi[j]} ${config[i]} ; }

        input_drawhist=rootfiles/${tag}_${tagki}/savehist.root
        [[ $run_drawhist -eq 1 ]] && { 
            [[ -f $input_drawhist ]] &&
            { ./mcana_drawhist.exe $input_drawhist $tag ; } ||
            { echo -e "\e[31;1merror:\e[0m no input file \e[4m$input_drawhist\e[0m." ; } }
    done
done

rm getfname.exe 2> /dev/null
rm mcana_savehist.exe 2> /dev/null
# rm mcana_drawhist.exe 2> /dev/null
