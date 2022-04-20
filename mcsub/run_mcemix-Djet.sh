#!/bin/bash

# jsyst: input, ishi, isembed, tag
input=(
    # /raid5/data/wangj/DntupleRun2018/djtgmt_20220330_Djet_20220323_DiJet_pThat-15_HydjetDrumMB_trk1Dpt4_1_10199_allgenjet_jtpt60.root,incl_
    /raid5/data/wangj/DntupleRun2018/djtgmt_20220330_Djet_20220323_DiJet_pThat-15_HydjetDrumMB_trk1Dpt4_1_10199_allgenjet_jtpt60_NMIX1_eta1p6.root,emix_
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
[[ $run_savehist -eq 1 || $# == 0 ]] && { g++ djmcemix_savehist.cc $(root-config --libs --cflags) -I"../includes/" -g -o djmcemix_savehist.exe || exit 1 ; }
[[ $run_drawhist -eq 1 || $# == 0 ]] && { g++ djmcemix_drawhist.cc $(root-config --libs --cflags) -I"../includes/" -g -o djmcemix_drawhist.exe || exit 1 ; }

for jinput in ${input[@]}
do
    IFS=','; inputs=(${jinput}); unset IFS;
    inputfile=${inputs[0]}
    ver=${inputs[1]}
    tag="mcemix_"${ver}

    if [[ $run_savehist -eq 1 ]]
    then
        for iconfig in "${config[@]}"
        do
            outputdir=${tag}_$(./getfname.exe $ishi ${iconfig} $ismc "Djet")
            echo -e "--> \e[32;7m$outputdir\e[0m"

            ./djmcemix_savehist.exe $inputfile $tag $ishi ${iconfig} $ismc &
        done
    fi
done

wait

if [[ $run_drawhist -eq 1 ]]
then
    for iconfig in "${config[@]}"
    do
        outputdir=$(./getfname.exe $ishi ${iconfig} $ismc "Djet")
        echo -e "--> \e[32;7m$outputdir\e[0m"
        ./djmcemix_drawhist.exe "rootfiles/mcemix_incl__"$outputdir"/savehist.root" "rootfiles/mcemix_emix__"$outputdir"/savehist.root" "rootfiles/mcsub__"$outputdir"/savehist.root" "mcclosure__"$outputdir
    done
fi

rm getfname.exe 2> /dev/null
rm djmcemix_savehist.exe 2> /dev/null
rm djmcemix_drawhist.exe 2> /dev/null
