#!/bin/bash

ikine=(19)
jsyst=(1)

# ikine
# Dptmin Dptmax Dymax centmin centmax jetptmin jetetamax
config=(
    "6 30 1.2 0 90 80 1.6"  # 0 default
    "2 10 1.2 0 90 80 1.6" # 1
    "2 15 1.2 0 90 80 1.6" # 2
    "2 20 1.2 0 90 80 1.6" # 3
    "2 25 1.2 0 90 80 1.6" # 4
    "2 30 1.2 0 90 80 1.6" # 5
    "2 50 1.2 0 90 80 1.6" # 6
    "2 100 1.2 0 90 80 1.6" # 7
    "3 10 1.2 0 90 80 1.6" # 8
    "3 15 1.2 0 90 80 1.6" # 9
    "3 20 1.2 0 90 80 1.6" # 10
    "3 25 1.2 0 90 80 1.6" # 11
    "3 30 1.2 0 90 80 1.6" # 12
    "3 50 1.2 0 90 80 1.6" # 13
    "3 100 1.2 0 90 80 1.6" # 14
    "4 10 1.2 0 90 80 1.6" # 15
    "4 15 1.2 0 90 80 1.6" # 16
    "4 20 1.2 0 90 80 1.6" # 17
    "4 25 1.2 0 90 80 1.6" # 18
    "4 30 1.2 0 90 80 1.6" # 19
    "4 50 1.2 0 90 80 1.6" # 20
    "4 100 1.2 0 90 80 1.6" # 21
    "5 10 1.2 0 90 80 1.6" # 22
    "5 15 1.2 0 90 80 1.6" # 23
    "5 20 1.2 0 90 80 1.6" # 24
    "5 25 1.2 0 90 80 1.6" # 25
    "5 30 1.2 0 90 80 1.6" # 26
    "5 50 1.2 0 90 80 1.6" # 27
    "5 100 1.2 0 90 80 1.6" # 28
    "6 10 1.2 0 90 80 1.6" # 29
    "6 15 1.2 0 90 80 1.6" # 30
    "6 20 1.2 0 90 80 1.6" # 31
    "6 25 1.2 0 90 80 1.6" # 32
    "6 30 1.2 0 90 80 1.6" # 33 default
    "6 50 1.2 0 90 80 1.6" # 34
    "6 100 1.2 0 90 80 1.6" # 35
    "10 15 1.2 0 90 80 1.6" # 36
    "10 20 1.2 0 90 80 1.6" # 37
    "10 25 1.2 0 90 80 1.6" # 38
    "10 30 1.2 0 90 80 1.6" # 39
    "10 50 1.2 0 90 80 1.6" # 40
    "10 100 1.2 0 90 80 1.6" # 41
    "15 20 1.2 0 90 80 1.6" # 42
    "15 25 1.2 0 90 80 1.6" # 43
    "15 30 1.2 0 90 80 1.6" # 44
    "15 50 1.2 0 90 80 1.6" # 45
    "15 100 1.2 0 90 80 1.6" # 46
    "20 25 1.2 0 90 80 1.6" # 47
    "20 30 1.2 0 90 80 1.6" # 48
    "20 50 1.2 0 90 80 1.6" # 49
    "20 100 1.2 0 90 80 1.6" # 50
    "25 30 1.2 0 90 80 1.6" # 51
    "25 50 1.2 0 90 80 1.6" # 52
    "25 100 1.2 0 90 80 1.6" # 53
    "30 50 1.2 0 90 80 1.6" # 54
    "30 100 1.2 0 90 80 1.6" # 55
    "50 100 1.2 0 90 80 1.6" # 56
)

# jsyst
# inputfile, ishi, tag, ismc, binning
input=(
    /raid5/data/wangj/DntupleRun2017/djtcut_20210322_Djet_20210315_HighEGJet_Run2017G_17Nov2017_HIAK4CaloJet80_trk1Dpt2_v2_jtpt70.root,0,trig,0,finebin
    /raid5/data/wangj/DntupleRun2018/djtmva_20210322_Djet_20210315_HIHardProbes_04Apr2019_PuAK4CaloJet80100Eta5p1_trk1Dpt2_jtpt70.root,1,trig,0,finebin
    /raid5/data/wangj/DntupleRun2018/djtmva_20210322_Djet_20210315_HIHardProbes_04Apr2019_PuAK4CaloJet80100Eta5p1_trk1Dpt2_jtpt70_NMIX4_jt80.root,1,emix,0,finebin
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
        
        [[ $run_savehist -eq 1 ]] && { ./djana_savehist.exe $inputfile $tag $ishi ${config[i]} $ismc ; } &
    done
    wait

    for i in ${ikine[@]}
    do
        tagki=$(./getfname.exe $ishi ${config[i]} $ismc "Djet")
        echo -e "--> \e[42m$tag, $tagki\e[0m"
        
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
