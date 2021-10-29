#!/bin/bash

# jsyst: input, ishi, isembed,tag
input=(
    /raid5/data/wangj/DntupleRun2017/djtgmt_20211021_Djet_20210325_QCD_pThat-15_Dijet_mc2017_realistic_forppRef5TeV_trk1Dpt2_v2_jtpt40.root,0,0,
    /raid5/data/wangj/DntupleRun2018/djtgmt_20211021_Djet_20210328_DiJet_pThat-15_HydjetDrumMB_trk1Dpt2_1_31425_jtpt30.root,1,1, 
)
# ikine
config=(
    # "6 30 1.2 0 90 80 1.6"
    # "30 999 1.2 0 90 80 1.6"
    "6 10 2.0 0 90 80 2.0"
    "10 20 2.0 0 90 80 2.0"
    "20 30 2.0 0 90 80 2.0"
    "30 50 2.0 0 90 80 2.0"
    "50 80 2.0 0 90 80 2.0"
    # "6 10 1.2 0 90 80 1.6"
    # "10 20 1.2 0 90 80 1.6"
    # "20 30 1.2 0 90 80 1.6"
    # "30 50 1.2 0 90 80 1.6"
    # "50 80 1.2 0 90 80 1.6"
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

for jinput in ${input[@]}
do
    IFS=','; inputs=(${jinput}); unset IFS;
    inputfile=${inputs[0]}
    ishi=${inputs[1]}
    isembed=${inputs[2]}
    ver=${inputs[3]}
    tag="mc_"${ver}${tags_isembed[$isembed]}

    if [[ $run_savehist -eq 1 ]]
    then
        for iconfig in "${config[@]}"
        do
            outputdir=${tag}_$(./getfname.exe $ishi ${iconfig} $ismc "Djet")
            echo -e "--> \e[32;7m$outputdir\e[0m"

            ./djtmc_savehist.exe $inputfile $tag $isembed $ishi ${iconfig} $ismc &
        done
    fi
done

wait

for jinput in ${input[@]}
do
    IFS=','; inputs=(${jinput}); unset IFS;
    ishi=${inputs[1]}
    isembed=${inputs[2]}
    ver=${inputs[3]}
    tag="mc_"${ver}${tags_isembed[$isembed]}

    if [[ $run_drawhist -eq 1 ]]
    then
        for iconfig in "${config[@]}"
        do        
            outputdir=${tag}_$(./getfname.exe $ishi ${iconfig} $ismc "Djet")
            echo -e "--> \e[32;7m$outputdir\e[0m"
            input_drawhist=rootfiles/${outputdir}/savehist.root

            [[ -f $input_drawhist ]] &&
            { ./djtmc_drawhist.exe $input_drawhist $tag ; } ||
            { echo -e "\e[31;1merror:\e[0m no input file \e[4m$input_drawhist\e[0m ." ; } 
        done
    fi
done

rm getfname.exe 2> /dev/null
rm djtmc_savehist.exe 2> /dev/null
rm djtmc_drawhist.exe 2> /dev/null
