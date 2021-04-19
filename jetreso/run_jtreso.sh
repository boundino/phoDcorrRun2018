#!/bin/bash

jsyst=(0 1)

# jsyst
# inputfile, ishi, tag
input=(
    /raid5/data/wangj/DntupleRun2017/djtgmt_20210326_Djet_20210325_QCD_pThat-15_Dijet_mc2017_realistic_forppRef5TeV_trk1Dpt2_v2_jtpt70.root,0,dijet15
    /raid5/data/wangj/DntupleRun2018/djtgmt_20210326_Djet_20210328_DiJet_pThat-15_HydjetDrumMB_trk1Dpt2_jtpt70.root,1,dijet15
)


##
run_savehist=${1:-0}
run_fithist=${2:-0}
run_drawhist=${3:-0}

##
tishi=("pp" "PbPb")
##
[[ $run_savehist -eq 1 || $# == 0 ]] && { g++ jtreso_savehist.cc $(root-config --libs --cflags) -I"../includes/" -g -o jtreso_savehist.exe || exit 1 ; }
[[ $run_fithist -eq 1 || $# == 0 ]] && { g++ jtreso_fithist.cc $(root-config --libs --cflags) -I"../includes/" -g -o jtreso_fithist.exe || exit 1 ; }
[[ $run_drawhist -eq 1 || $# == 0 ]] && { g++ jtreso_drawhist.cc $(root-config --libs --cflags) -I"../includes/" -g -o jtreso_drawhist.exe || exit 1 ; }

for j in ${jsyst[@]}
do
    IFS=','; inputs=(${input[j]}); unset IFS;
    inputfile=${inputs[0]}
    ishi=${inputs[1]}
    tag="jtreso_${inputs[2]}"
    tagki=${tishi[$ishi]}

    echo -e "--> \e[42m$tag, $tagki\e[0m"
    
    [[ $run_savehist -eq 1 ]] && { ./jtreso_savehist.exe $inputfile $tag $ishi ; } & 

    wait

    input_fithist=rootfiles/${tag}_${tagki}/savehist.root
    [[ $run_fithist -eq 1 ]] && {
        [[ -f $input_fithist ]] && 
        { ./jtreso_fithist.exe $input_fithist $tag ; } ||
        { echo -e "\e[31;1merror:\e[0m no input file \e[4m$input_fithist\e[0m." ; } }

    input_drawhist=rootfiles/${tag}_${tagki}/fithist.root
    [[ $run_drawhist -eq 1 ]] && {
        [[ -f $input_drawhist ]] && 
        { ./jtreso_drawhist.exe $input_drawhist $tag ; } ||
        { echo -e "\e[31;1merror:\e[0m no input file \e[4m$input_drawhist\e[0m." ; } }
done

rm jtreso_savehist.exe > /dev/null 2>&1
rm jtreso_fithist.exe > /dev/null 2>&1
rm jtreso_drawhist.exe > /dev/null 2>&1
