#!/bin/bash

ikine=(0)
jsyst=(0 1 2)

# ikine
# Dptmin Dptmax Dymax centmin centmax phoptmin phoetamax
config=(
    "5 100 1.2 0 90 40 1.442"  # 0 default
    "2 100 1.2 0 90 20 1.442"  # 1
    "3 100 1.2 0 90 20 1.442"  # 2
    "5 100 1.2 0 90 20 1.442"  # 3
    "2 100 1.2 0 90 20 2.4"    # 4
    "5 100 1.2 0 30 40 1.442"  # 5
    "5 100 1.2 30 90 40 1.442"  # 6
)

# binning
tagpri="ana_finebin"

# jsyst
# inputfile, ishi, tag, ismc
input=(
    /raid5/data/wangj/DntupleRun2017/phodcut_20201002_Dpho_20201002_HighEGJet_17Nov2017_HIPhoton_HoverELoose_trk1Dpt2_vfl2.root,0,trig,0
    /raid5/data/wangj/DntupleRun2018/phodmva_20200924_Dpho_20200921_HIHardProbes_04Apr2019_HIGEDPhoton40_trk1Dpt2.root,1,trig,0
    /raid5/data/wangj/DntupleRun2018/phodmva_20210217_Dpho_20200921_HIHardProbes_04Apr2019_HIGEDPhoton40_trk1Dpt2_phoEt35_hlt40_NMIX40_photight.root,1,emix,0
    /raid5/data/wangj/DntupleRun2018/phodmva_20210201_Dpho_20210120_HIMinimumBias_trk1Dpt2_phoEt19_rm0size_f171113.root,1,mb27p,0
)

##
run_savehist=${1:-0}
run_fithist=${2:-0}
run_calchist=${3:-0}

##
g++ getfname.cc -I"../includes/" $(root-config --libs --cflags) -g -o getfname.exe || { rm *.exe 2> /dev/null ; exit 1 ; }
[[ $run_savehist -eq 1 || $# == 0 ]] && { g++ pdana_savehist.cc $(root-config --libs --cflags) -I"../includes/" -g -o pdana_savehist.exe || exit 1 ; }
[[ $run_fithist -eq 1 || $# == 0 ]] && { g++ pdana_fithist.cc $(root-config --libs --cflags) -I"../includes/" -g -o pdana_fithist.exe || exit 1 ; }
[[ $run_calchist -eq 1 || $# == 0 ]] && { g++ pdana_calchist.cc $(root-config --libs --cflags) -I"../includes/" -g -o pdana_calchist.exe || exit 1 ; }

for j in ${jsyst[@]}
do
    IFS=','; inputs=(${input[j]}); unset IFS;
    inputfile=${inputs[0]}
    ishi=${inputs[1]}
    tag_sample=${inputs[2]}
    ismc=${inputs[3]}
    tag=${tagpri}_${tag_sample}

    for i in ${ikine[@]}
    do
        tagki=$(./getfname.exe $ishi ${config[i]} $ismc)
        echo -e "--> \e[42m$tag, $tagki\e[0m"
        
        [[ $run_savehist -eq 1 ]] && { ./pdana_savehist.exe $inputfile $tag $ishi ${config[i]} $ismc ; }

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
