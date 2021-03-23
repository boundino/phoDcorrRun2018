#!/bin/bash

ikine=(5 6 7 8)
jsyst=(1)

# ikine
# Dptmin Dptmax Dymax centmin centmax jetptmin jetetamax
config=(
    "5 100 1.2 0 90 80 1.6"  # 0 default
    "2 5 1.2 0 90 80 1.6"    # 1
    "5 10 1.2 0 90 80 1.6"   # 2
    "10 20 1.2 0 90 80 1.6"  # 3
    "20 100 1.2 0 90 80 1.6" # 4
    "5 100 1.2 0 10 80 1.5"  # 5
    "5 100 1.2 10 30 80 1.5"  # 6
    "5 100 1.2 30 50 80 1.5" # 7
    "5 100 1.2 50 90 80 1.5" # 8
)

# jsyst
# inputfile, ishi, tag, ismc, binning
input=(
    /raid5/data/wangj/DntupleRun2017/djtcut_20210322_Djet_20210315_HighEGJet_Run2017G_17Nov2017_HIAK4CaloJet80_trk1Dpt2_v2_jtpt70.root,0,trig,0,finebin
    /raid5/data/wangj/DntupleRun2018/djtmva_20210322_Djet_20210315_HIHardProbes_04Apr2019_PuAK4CaloJet80100Eta5p1_trk1Dpt2_jtpt70.root,1,trig,0,finebin
)

##
run_savehist=${1:-0}
run_fithist=${2:-0}
run_calchist=${3:-0}

##
g++ getfname.cc -I"../includes/" $(root-config --libs --cflags) -g -o getfname.exe || { rm *.exe 2> /dev/null ; exit 1 ; }
[[ $run_savehist -eq 1 || $# == 0 ]] && { g++ djana_savehist.cc $(root-config --libs --cflags) -I"../includes/" -g -o djana_savehist.exe || exit 1 ; }
[[ $run_fithist -eq 1 || $# == 0 ]] && { g++ djana_fithist.cc $(root-config --libs --cflags) -I"../includes/" -g -o djana_fithist.exe || exit 1 ; }
[[ $run_calchist -eq 1 || $# == 0 ]] && { g++ djana_calchist.cc $(root-config --libs --cflags) -I"../includes/" -g -o djana_calchist.exe || exit 1 ; }

for j in ${jsyst[@]}
do
    IFS=','; inputs=(${input[j]}); unset IFS;
    inputfile=${inputs[0]}
    ishi=${inputs[1]}
    tag_sample=${inputs[2]}
    ismc=${inputs[3]}
    tagpri="ana_${inputs[4]}"
    tag=${tagpri}_${tag_sample}

    for i in ${ikine[@]}
    do
        tagki=$(./getfname.exe $ishi ${config[i]} $ismc "Djet")
        echo -e "--> \e[42m$tag, $tagki\e[0m"
        
        [[ $run_savehist -eq 1 ]] && { ./djana_savehist.exe $inputfile $tag $ishi ${config[i]} $ismc ; }

        input_fithist=rootfiles/${tag}_${tagki}/savehist.root
        [[ $run_fithist -eq 1 ]] && {
            [[ -f $input_fithist ]] && 
            { ./djana_fithist.exe $input_fithist $tag ; } ||
            { echo -e "\e[31;1merror:\e[0m no input file \e[4m$input_fithist\e[0m." ; } }

        input_calchist=rootfiles/${tag}_${tagki}/fithist.root
        [[ $run_calchist -eq 1 ]] && {
            [[ -f $input_calchist ]] && 
            { ./djana_calchist.exe $input_calchist $tag ; } ||
            { echo -e "\e[31;1merror:\e[0m no input file \e[4m$input_calchist\e[0m." ; } }

    done
done

rm getfname.exe > /dev/null 2>&1
rm djana_savehist.exe > /dev/null 2>&1
rm djana_fithist.exe > /dev/null 2>&1
rm djana_calchist.exe > /dev/null 2>&1
