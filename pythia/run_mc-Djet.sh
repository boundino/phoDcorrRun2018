#!/bin/bash

jsyst=(0)
ikine=(2)

###
# jsyst: input, ishi, isembed
input=(
    /raid5/data/wangj/DntupleRun2017/djtgmt_20210326_Djet_20210325_QCD_pThat-15_Dijet_mc2017_realistic_forppRef5TeV_trk1Dpt2_v2_jtpt70.root,0,0 # 0
    /raid5/data/wangj/DntupleRun2018/djtgmt_20210326_Djet_20210328_DiJet_pThat-15_HydjetDrumMB_trk1Dpt2_jtpt70.root,1,1 # 1
)
# ikine
config=(
    "6 30 1.2 0 90 80 1.6"  # 0
    "30 999 1.2 0 90 80 1.6"   # 1
    "4 30 1.2 0 90 80 1.6"  # 2
)

########

##
ismc=1
tags_isembed=("pythia" "embed")

##
run_savehist=${1:-0}
run_drawhist=${2:-0}

##
g++ getfname.cc -I"../includes/" $(root-config --libs --cflags) -g -o getfname.exe || { rm *.exe 2> /dev/null ; exit 1 ; }
[[ $run_savehist -eq 1 || $# == 0 ]] && { g++ djtmc_savehist.cc $(root-config --libs --cflags) -I"../includes/" -g -o djtmc_savehist.exe || exit 1 ; }
[[ $run_drawhist -eq 1 || $# == 0 ]] && { g++ djtmc_drawhist.cc $(root-config --libs --cflags) -I"../includes/" -g -o djtmc_drawhist.exe || exit 1 ; }

for j in ${jsyst[@]}
do
    IFS=','; inputs=(${input[j]}); unset IFS;
    inputfile=${inputs[0]}
    ishi=${inputs[1]}
    isembed=${inputs[2]}
    tag="mc_"${tags_isembed[$isembed]}

    for i in ${ikine[@]}
    do
        outputdir=${tag}_$(./getfname.exe $ishi ${config[i]} $ismc "Djet")
        echo -e "--> \e[42m$outputdir\e[0m"

        [[ $run_savehist -eq 1 ]] && { ./djtmc_savehist.exe $inputfile $tag $isembed $ishi ${config[i]} $ismc ; } &
    done
    wait

    for i in ${ikine[@]}
    do        
        outputdir=${tag}_$(./getfname.exe $ishi ${config[i]} $ismc "Djet")
        echo -e "--> \e[42m$outputdir\e[0m"
        input_drawhist=rootfiles/${outputdir}/savehist.root

        [[ $run_drawhist -eq 1 ]] && { 
            [[ -f $input_drawhist ]] &&
            { ./djtmc_drawhist.exe $input_drawhist $tag $isembed; } ||
            { echo -e "\e[31;1merror:\e[0m no input file \e[4m$input_drawhist\e[0m." ; } 
        }
    done
done

rm getfname.exe 2> /dev/null
rm djtmc_savehist.exe 2> /dev/null
rm djtmc_drawhist.exe 2> /dev/null
