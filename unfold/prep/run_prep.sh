#!/bin/bash

jsyst=(0 1)
ikine=(0 1)

# jsyst: input, ishi, inputdata
input=(
    `#0: pp  ` /raid5/data/wangj/DntupleRun2017/djtgmt_20210326_Djet_20210325_QCD_pThat-15_Dijet_mc2017_realistic_forppRef5TeV_trk1Dpt2_v2_jtpt70.root,0
    `#1: PbPb` /raid5/data/wangj/DntupleRun2018/djtgmt_20210326_Djet_20210328_DiJet_pThat-15_HydjetDrumMB_trk1Dpt2_jtpt70.root,1
)
# ikine
config=(
    `#0` "6 30 1.2 0 90 80 1.6" 
    `#1` "30 999 1.2 0 90 80 1.6"
)


##
run_savehist=${1:-0}

##
g++ getfname.cc -I"../../includes/" $(root-config --libs --cflags) -g -o getfname.exe || { rm *.exe 2> /dev/null ; exit 1 ; }
[[ $run_savehist -gt 0 || $# == 0 ]] && { g++ prep_savehist.cc $(root-config --libs --cflags) -I"../../includes/" -g -o prep_savehist.exe || exit 1 ; }

for j in ${jsyst[@]}
do
    IFS=','; inputs=(${input[j]}); unset IFS;
    inputfile=${inputs[0]}
    ishi=${inputs[1]}

    for i in ${ikine[@]}
    do
        outputdir="unfold_"$(./getfname.exe $ishi ${config[i]} "2"`#(ismc)` "Djet")
        echo -e "--> \e[32;7m$outputdir\e[0m"
        inputdata="../../mainana/rootfiles/comp_finebin_"$(./getfname.exe "2"`#(ishi)` ${config[i]} "2"`#(ismc)` "Djet")"/drawhist.root"
        ls $inputdata
        [[ $run_savehist -eq 1 ]] && { ./prep_savehist.exe $inputfile $inputdata $outputdir $ishi ${config[i]} ; } &
        [[ $run_savehist -eq 2 ]] && { ./prep_savehist.exe "rootfiles2/"$outputdir"_dphi.root" "rootfiles"; 
            ./prep_savehist.exe "rootfiles2/"$outputdir"_dr.root" "rootfiles"; }
    done
done
wait

rm getfname.exe 2> /dev/null
rm prep_savehist.exe 2> /dev/null
