#!/bin/bash

jsyst=(0 1 2)
ikine=(2)

###
ismc=1
# jsyst: input, ishi, isembed
input=(
    /raid5/data/wangj/DntupleRun2017/phodgmatch_20201104_Dpho_20201104_QCDPhoton_mc2017_realistic_forppRef5TeV_trk1Dpt2_pthatweight.root,0,0    # 0
    /raid5/data/wangj/DntupleRun2018/phodgmatch_20201019_Dpho_20200924_QCDPhoton_Filter30GeV_TuneCP5_HydjetDrumMB_trk1Dpt2_pthatweight.root,1,0 # 1
    /raid5/data/wangj/DntupleRun2018/phodgmatch_20201019_Dpho_20200924_QCDPhoton_Filter30GeV_TuneCP5_HydjetDrumMB_trk1Dpt2_pthatweight.root,1,1 # 2
)
# ikine
config=(
    "5 100 1.2 0 90 40 1.442"  # 0
    "5 100 2.4 0 90 40 2.4"  # 1
    "4 10 1.2 0 90 40 1.442"  # 2
)

##
tags_isembed=("pythia" "embed")

##
run_savehist=${1:-0}
run_drawhist=${2:-0}

##
g++ getfname.cc -I"../includes/" $(root-config --libs --cflags) -g -o getfname.exe || { rm *.exe 2> /dev/null ; exit 1 ; }
[[ $run_savehist -eq 1 || $# == 0 ]] && { g++ mcana_savehist.cc $(root-config --libs --cflags) -I"../includes/" -g -o mcana_savehist.exe || exit 1 ; }
[[ $run_drawhist -eq 1 || $# == 0 ]] && { g++ mcana_drawhist.cc $(root-config --libs --cflags) -I"../includes/" -g -o mcana_drawhist.exe || exit 1 ; }

for j in ${jsyst[@]}
do
    IFS=','; inputs=(${input[j]}); unset IFS;
    inputfile=${inputs[0]}
    ishi=${inputs[1]}
    isembed=${inputs[2]}
    tag="mc_"${tags_isembed[$isembed]}

    for i in ${ikine[@]}
    do
        outputdir=${tag}_$(./getfname.exe $ishi ${config[i]} $ismc)
        echo -e "--> \e[42m$outputdir\e[0m"

        set -x
        [[ $run_savehist -eq 1 ]] && { ./mcana_savehist.exe $inputfile $tag $isembed $ishi ${config[i]} $ismc ; }
        set +x
        
        input_drawhist=rootfiles/${outputdir}/savehist.root
        [[ $run_drawhist -eq 1 ]] && { 
            [[ -f $input_drawhist ]] &&
            { ./mcana_drawhist.exe $input_drawhist $tag $isembed; } ||
            { echo -e "\e[31;1merror:\e[0m no input file \e[4m$input_drawhist\e[0m." ; } 
        }
    done
done

rm getfname.exe 2> /dev/null
rm mcana_savehist.exe 2> /dev/null
rm mcana_drawhist.exe 2> /dev/null
