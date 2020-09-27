#!/bin/bash

ikine=(0)

#
ptmin=(2 3 4 5)
ptmax=(100 100 100 100)
ymax=(1.2 1.2 1.2 1.2)

centmin=0
centmax=90

# tag="prel"
tag="prel_finebin"

input=/raid5/data/wangj/DntupleRun2018/phodmva_20200926_Dpho_20200924_QCDPhoton_Filter30GeV_TuneCP5_HydjetDrumMB_trk1Dpt2_pthatweight.root
ishi=1

##
run_savehist=${1:-0}
# run_drawhist=${2:-0}

##
g++ getfname.cc -I"../includes/" $(root-config --libs --cflags) -g -o getfname.exe || { rm *.exe 2> /dev/null ; exit 1 ; }
[[ $run_savehist -eq 1 || $# == 0 ]] && { g++ mcana_savehist.cc $(root-config --libs --cflags) -I"../includes/" -g -o mcana_savehist.exe || exit 1 ; }
# [[ $run_drawhist -eq 1 || $# == 0 ]] && { g++ mcana_drawhist.cc $(root-config --libs --cflags) -I"../includes/" -g -o mcana_drawhist.exe || exit 1 ; }

for i in ${ikine[@]}
do
    tagki=$(./getfname.exe $ishi ${ptmin[i]} ${ptmax[i]} ${ymax[i]} $centmin $centmax)

    [[ $run_savehist -eq 1 ]] && { ./mcana_savehist.exe $input $tag $ishi ${ptmin[i]} ${ptmax[i]} ${ymax[i]} $centmin $centmax ; }
    input_drawhist=rootfiles/${tag}_${tagki}/savehist.root
    # [[ $run_drawhist -eq 1 ]] && { ./mcana_drawhist.exe $input_drawhist $tag ; }
done

rm getfname.exe
[[ -f mcana_savehist.exe ]] && rm mcana_savehist.exe
# [[ -f mcana_drawhist.exe ]] && rm mcana_drawhist.exe

