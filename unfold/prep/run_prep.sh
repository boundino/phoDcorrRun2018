#!/bin/bash

# jsyst: input, ishi, inputdata
input=(
    `#0: pp  ` /raid5/data/wangj/DntupleRun2017/djtgmt_20211021_Djet_20210325_QCD_pThat-15_Dijet_mc2017_realistic_forppRef5TeV_trk1Dpt2_v2_jtpt40.root,0
    `#1: PbPb` /raid5/data/wangj/DntupleRun2018/djtgmt_20211021_Djet_20210328_DiJet_pThat-15_HydjetDrumMB_trk1Dpt2_1_31425_jtpt30.root,1
    # `#0: pp  ` /raid5/data/wangj/DntupleRun2017/djtgmt_20210326_Djet_20210325_QCD_pThat-15_Dijet_mc2017_realistic_forppRef5TeV_trk1Dpt2_v2_jtpt70.root,0
    # `#1: PbPb` /raid5/data/wangj/DntupleRun2018/djtgmt_20210326_Djet_20210328_DiJet_pThat-15_HydjetDrumMB_trk1Dpt2_jtpt70.root,1
)
# ikine
config=(
    "6 10 1.2 0 90 80 1.6" 
    "10 20 1.2 0 90 80 1.6" 
    "20 30 1.2 0 90 80 1.6"
    "30 50 1.2 0 90 80 1.6"
    "50 80 1.2 0 90 80 1.6"
    # "6 30 1.2 0 90 80 1.6" 
    # "30 999 1.2 0 90 80 1.6"
)
# kvar
vars=(
    `#0` "dphi"
    `#1` "dr"
)

##
run_savehist=${1:-0}

##
g++ getfname.cc -I"../../includes/" $(root-config --libs --cflags) -g -o getfname.exe || { rm *.exe 2> /dev/null ; exit 1 ; }
[[ $run_savehist -gt 0 || $# == 0 ]] && { g++ prep_savehist.cc $(root-config --libs --cflags) -I"../../includes/" -g -o prep_savehist.exe || exit 1 ; }

for jinput in ${input[@]}
do
    IFS=','; inputs=(${jinput}); unset IFS;
    inputfile=${inputs[0]}
    ishi=${inputs[1]}

    for iconfig in "${config[@]}"
    do
        outputdir="unfold_"$(./getfname.exe $ishi ${iconfig} "2"`#(ismc)` "Djet")
        echo -e "--> \e[32;7m$outputdir\e[0m"
        inputdata="../../mainana/rootfiles/comp_finebin_"$(./getfname.exe "1"`#(ishi)` ${iconfig} "2"`#(ismc)` "Djet")"/drawhist.root"
        ls -l $inputdata
        
        for kvars in ${vars[@]}
        do
            [[ $run_savehist -eq 1 ]] && { ./prep_savehist.exe $inputfile $inputdata $outputdir $ishi ${iconfig} ${kvars}; } &
            # [[ $run_savehist -eq 2 ]] && { ./prep_savehist.exe "rootfiles2/"$outputdir"_dphi.root" "rootfiles"; 
            #     ./prep_savehist.exe "rootfiles2/"$outputdir"_dr.root" "rootfiles"; }
        done
    done
done
wait

rm getfname.exe 2> /dev/null
rm prep_savehist.exe 2> /dev/null
