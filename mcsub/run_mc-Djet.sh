#!/bin/bash

# jsyst: input, ishi, isembed,tag
input=(
    /raid5/data/wangj/DntupleRun2018/djtgmt_20211111_Djet_20211107_DiJet_pThat-15_HydjetDrumMB_trk1Dpt4_1_31425_jtpt20.root,old_
    # /raid5/data/wangj/DntupleRun2018/djtgmt_20211111_Djet_20211107_DiJet_pThat-15_HydjetDrumMB_trk1Dpt4_1_31425_jtpt20.root,noweight_
    # /raid5/data/wangj/DntupleRun2018/djtgmt_20211111_Djet_20211107_DiJet_pThat-15_HydjetDrumMB_trk1Dpt4_1_31425_jtpt20.root,tightmatch_
    # /raid5/data/wangj/DntupleRun2018/djtgmt_20220330_Djet_20220323_DiJet_pThat-15_HydjetDrumMB_trk1Dpt4_1_10199_allgenjet_jtpt60.root,
)
# ikine
config=(
    "4 20 1.2 0 90 60 1.6"
    "20 999 1.2 0 90 60 1.6"
    # "4 20 1.2 0 30 60 1.6"
    # "20 999 1.2 0 30 60 1.6"
    # "4 20 1.2 30 50 60 1.6"
    # "20 999 1.2 30 50 60 1.6"
    # "4 20 1.2 50 90 60 1.6"
    # "20 999 1.2 50 90 60 1.6"
)

########

ishi=1
ismc=1
##
run_savehist=${1:-0}
run_drawhist=${2:-0}

##
g++ getfname.cc -I"../includes/" $(root-config --libs --cflags) -g -o getfname.exe || { rm *.exe 2> /dev/null ; exit 1 ; }
[[ $run_savehist -eq 1 || $# == 0 ]] && { g++ djmcsub_savehist.cc $(root-config --libs --cflags) -I"../includes/" -g -o djmcsub_savehist.exe || exit 1 ; }
[[ $run_drawhist -eq 1 || $# == 0 ]] && { g++ djmcsub_drawhist.cc $(root-config --libs --cflags) -I"../includes/" -g -o djmcsub_drawhist.exe || exit 1 ; }

for jinput in ${input[@]}
do
    IFS=','; inputs=(${jinput}); unset IFS;
    inputfile=${inputs[0]}
    ver=${inputs[1]}
    tag="mcsub_"${ver}

    if [[ $run_savehist -eq 1 ]]
    then
        for iconfig in "${config[@]}"
        do
            outputdir=${tag}_$(./getfname.exe $ishi ${iconfig} $ismc "Djet")
            echo -e "--> \e[32;7m$outputdir\e[0m"

            ./djmcsub_savehist.exe $inputfile $tag $ishi ${iconfig} $ismc &
        done
    fi
done

wait

for jinput in ${input[@]}
do
    IFS=','; inputs=(${jinput}); unset IFS;
    ver=${inputs[1]}
    tag="mcsub_"${ver}

    if [[ $run_drawhist -eq 1 ]]
    then
        for iconfig in "${config[@]}"
        do        
            outputdir=${tag}_$(./getfname.exe $ishi ${iconfig} $ismc "Djet")
            echo -e "--> \e[32;7m$outputdir\e[0m"
            input_drawhist=rootfiles/${outputdir}/savehist.root

            [[ -f $input_drawhist ]] &&
            { ./djmcsub_drawhist.exe $input_drawhist $tag ; } ||
            { echo -e "\e[31;1merror:\e[0m no input file \e[4m$input_drawhist\e[0m ." ; } 
        done
    fi
done

rm getfname.exe 2> /dev/null
rm djmcsub_savehist.exe 2> /dev/null
rm djmcsub_drawhist.exe 2> /dev/null
