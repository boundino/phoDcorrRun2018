#!/bin/bash

ikine=(0 1 2 3)

#
ptmin=(2 3 4 5)
ptmax=(100 100 100 100)
ymax=(2 2 2 2)

centmin=0
centmax=90

# tag="prel"
tag="prel_finebin"

input=/raid5/data/wangj/DntupleRun2018/phodmva_20200924_crab_Dpho_20200921_HIHardProbes_04Apr2019_HIGEDPhoton40_trk1Dpt2.root
ishi=1

##
run_savehist=${1:-0}
run_fithist=${2:-0}

##
g++ getfname.cc -I"../includes/" $(root-config --libs --cflags) -g -o getfname.exe || { rm *.exe 2> /dev/null ; exit 1 ; }
[[ $run_savehist -eq 1 || $# == 0 ]] && { g++ pdana_savehist.cc $(root-config --libs --cflags) -I"../includes/" -g -o pdana_savehist.exe || exit 1 ; }
[[ $run_fithist -eq 1 || $# == 0 ]] && { g++ pdana_fithist.cc $(root-config --libs --cflags) -I"../includes/" -g -o pdana_fithist.exe || exit 1 ; }

for i in ${ikine[@]}
do
    tagki=$(./getfname.exe $ishi ${ptmin[i]} ${ptmax[i]} ${ymax[i]} $centmin $centmax)

    [[ $run_savehist -eq 1 ]] && { ./pdana_savehist.exe $input $tag $ishi ${ptmin[i]} ${ptmax[i]} ${ymax[i]} $centmin $centmax ; }
    input_fithist=rootfiles/${tag}_${tagki}/savehist.root
    [[ $run_fithist -eq 1 ]] && { ./pdana_fithist.exe $input_fithist $tag ; }
done

rm getfname.exe
[[ -f pdana_savehist.exe ]] && rm pdana_savehist.exe
[[ -f pdana_fithist.exe ]] && rm pdana_fithist.exe

