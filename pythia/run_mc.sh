#!/bin/bash

jsyst=(0 1) # ishi
ikine=(0)
kcut=(1)
ismc=1

# jsyst
input=(
    /raid5/data/wangj/DntupleRun2017/phodgmatch_20201104_Dpho_20201104_QCDPhoton_mc2017_realistic_forppRef5TeV_trk1Dpt2_pthatweight.root # pp
    /raid5/data/wangj/DntupleRun2018/phodgmatch_20201019_Dpho_20200924_QCDPhoton_Filter30GeV_TuneCP5_HydjetDrumMB_trk1Dpt2_pthatweight.root # PbPb
)
ishi=(0 1)
# ikine
config=(
    "5 100 1.2 0 90 40 1.442"  # 0
)
#kcut
tag=(
    "prel_default 0"
    "prel_recoiso 1"
    "prel_hydjet 4"
)

##
run_savehist=${1:-0}
run_drawhist=${2:-0}

##
g++ getfname.cc -I"../includes/" $(root-config --libs --cflags) -g -o getfname.exe || { rm *.exe 2> /dev/null ; exit 1 ; }
[[ $run_savehist -eq 1 || $# == 0 ]] && { g++ mcana_savehist.cc $(root-config --libs --cflags) -I"../includes/" -g -o mcana_savehist.exe || exit 1 ; }
[[ $run_drawhist -eq 1 || $# == 0 ]] && { g++ mcana_drawhist.cc $(root-config --libs --cflags) -I"../includes/" -g -o mcana_drawhist.exe || exit 1 ; }

for i in ${ikine[@]}
do
    for j in ${jsyst[@]}
    do
        for k in ${kcut[@]}
        do
            tagki=$(./getfname.exe ${ishi[j]} ${config[i]} $ismc)
            IFS=' '; tagstr=(${tag[k]}); unset IFS;
            set -x
            [[ $run_savehist -eq 1 ]] && { ./mcana_savehist.exe ${input[j]} ${tag[k]} ${ishi[j]} ${config[i]} $ismc ; }
            set +x

            input_drawhist=rootfiles/${tagstr[0]}_${tagki}/savehist.root
            [[ $run_drawhist -eq 1 ]] && { 
                [[ -f $input_drawhist ]] &&
                { ./mcana_drawhist.exe $input_drawhist ${tag[k]} ; } ||
                { echo -e "\e[31;1merror:\e[0m no input file \e[4m$input_drawhist\e[0m." ; } }
        done
    done
done

rm getfname.exe 2> /dev/null
rm mcana_savehist.exe 2> /dev/null
rm mcana_drawhist.exe 2> /dev/null
