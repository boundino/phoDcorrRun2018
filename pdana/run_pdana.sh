#!/bin/bash

ikine=(0)
jsyst=(0) # ishi

#
# Dptmin Dptmax Dymax centmin centmax phoptmin phoetamax
config=(
    "2 100 1.2 0 90 40 1.442"
    "2 5 1.2 0 90 40 1.442"
    "5 100 1.2 0 90 40 1.442"
    "5 100 1.2 0 90 50 1.442"
    "5 100 1.2 0 90 60 1.442"
    "5 100 1.2 0 90 80 1.442"
    "5 100 1.2 0 90 100 1.442"
)

# tag="prel_lessbin"
tag="prel_finebin"

input=(
    /raid5/data/wangj/DntupleRun2017/phodcut_20201002_Dpho_20201002_HighEGJet_17Nov2017_HIPhoton_HoverELoose_trk1Dpt2_vfl2.root
    /raid5/data/wangj/DntupleRun2018/phodmva_20200924_Dpho_20200921_HIHardProbes_04Apr2019_HIGEDPhoton40_trk1Dpt2.root
)
ishi=(0 1)

##
run_savehist=${1:-0}
run_fithist=${2:-0}
run_calchist=${3:-0}

##
g++ getfname.cc -I"../includes/" $(root-config --libs --cflags) -g -o getfname.exe || { rm *.exe 2> /dev/null ; exit 1 ; }
[[ $run_savehist -eq 1 || $# == 0 ]] && { g++ pdana_savehist.cc $(root-config --libs --cflags) -I"../includes/" -g -o pdana_savehist.exe || exit 1 ; }
[[ $run_fithist -eq 1 || $# == 0 ]] && { g++ pdana_fithist.cc $(root-config --libs --cflags) -I"../includes/" -g -o pdana_fithist.exe || exit 1 ; }
[[ $run_calchist -eq 1 || $# == 0 ]] && { g++ pdana_calchist.cc $(root-config --libs --cflags) -I"../includes/" -g -o pdana_calchist.exe || exit 1 ; }

for i in ${ikine[@]}
do
    for j in ${jsyst[@]}
    do
        tagki=$(./getfname.exe ${ishi[j]} ${config[i]})
        
        [[ $run_savehist -eq 1 ]] && { ./pdana_savehist.exe ${input[j]} $tag ${ishi[j]} ${config[i]} ; }

        input_fithist=rootfiles/${tag}_${tagki}/savehist.root
        [[ $run_fithist -eq 1 ]] && {
            [[ -f $input_fithist ]] && 
            { ./pdana_fithist.exe $input_fithist $tag ; } ||
            { echo -e "\e[31;1merror:\e[0m no input file \e[4m$input_fithist\e[0m." ; } }

        input_calchist=rootfiles/${tag}_${tagki}/fithist.root
        [[ $run_calchist -eq 1 ]] && {
            [[ -f $input_calchist ]] && 
            { ./pdana_calchist.exe $input_calchist $tag ; } ||
            { echo -e "\e[31;1merror:\e[0m no input file \e[4m$input_calchist\e[0m." ; } }

    done
done

rm getfname.exe > /dev/null 2>&1
rm pdana_savehist.exe > /dev/null 2>&1
rm pdana_fithist.exe > /dev/null 2>&1
rm pdana_calchist.exe > /dev/null 2>&1
