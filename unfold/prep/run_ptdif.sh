#!/bin/bash

# jsyst: input, ishi, isNcoll
input=(
    `#0: pp  ` /raid5/data/wangj/DntupleRun2017/djtgmt_20211021_Djet_20210325_QCD_pThat-15_Dijet_mc2017_realistic_forppRef5TeV_trk1Dpt2_v2_jtpt40.root,0,0
    `#1: PbPb` /raid5/data/wangj/DntupleRun2018/djtgmt_20211021_Djet_20210328_DiJet_pThat-15_HydjetDrumMB_trk1Dpt2_1_31425_jtpt30.root,1,1
)
vars=(
    `#0` "dphi"
    `#1` "dr"
)

##
run_savehist=${1:-0}
run_drawhist=${2:-0}

tishi=("_pp" "_PbPb") ; tisNcoll=("" "_wNcoll");

##
[[ $run_savehist -gt 0 || $# == 0 ]] && { g++ ptdif_savehist.cc $(root-config --libs --cflags) -I"../../includes/" -g -o ptdif_savehist.exe || exit 1 ; }
[[ $run_drawhist -gt 0 || $# == 0 ]] && { g++ ptdif_drawhist.cc $(root-config --libs --cflags) -I"../../includes/" -g -o ptdif_drawhist.exe || exit 1 ; }

for jinput in ${input[@]}
do
    IFS=','; inputs=(${jinput}); unset IFS;
    inputfile=${inputs[0]}
    ishi=${inputs[1]}
    isNcoll=${inputs[2]}

    outputdir="ptdif"${tishi[$ishi]}${tisNcoll[$isNcoll]}
    echo -e "--> \e[32;7m$outputdir\e[0m"
    
    for kvars in ${vars[@]}
    do
        [[ $run_savehist -eq 1 ]] && { ./ptdif_savehist.exe $inputfile $outputdir $kvars $ishi $isNcoll; } &
    done
done
wait

for jinput in ${input[@]}
do
    IFS=','; inputs=(${jinput}); unset IFS;
    inputfile=${inputs[0]}
    ishi=${inputs[1]}
    isNcoll=${inputs[2]}

    outputdir="ptdif"${tishi[$ishi]}${tisNcoll[$isNcoll]}
    echo -e "--> \e[32;7m$outputdir\e[0m"
    
    for kvars in ${vars[@]}
    do
        [[ $run_drawhist -eq 1 ]] && { ./ptdif_drawhist.exe "rootfiles_checks/"$outputdir"_"$kvars".root" $outputdir $kvars ; }
    done
done

rm ptdif_savehist.exe 2> /dev/null
rm ptdif_drawhist.exe 2> /dev/null
