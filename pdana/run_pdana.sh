#!/bin/bash

ptmin=2
ptmax=5
ymax=1.2
centmin=0
centmax=90

tag="prel"
# tag="prel_finebin"

input=/raid5/data/wangj/DntupleRun2018/phod_20200923_crab_Dpho_20200921_HIHardProbes_04Apr2019_HIGEDPhoton40_trk1Dpt2.root

##
g++ getfname.cc -I"../includes/" $(root-config --libs --cflags) -g -o getfname_${tmp}.exe || { rm *_${tmp}.exe 2> /dev/null ; exit 1 ; }
tagki=$(./getfname_${tmp}.exe $ptmin $ptmax $ymax $centmin $centmax)
rm getfname_${tmp}.exe

##
run_savehist=${1:-0}
run_fithist=${2:-0}

[[ $run_savehist -eq 1 || $# == 0 ]] && { g++ pdana_savehist.cc $(root-config --libs --cflags) -I"../includes/" -g -o pdana_savehist.exe || exit 1 ; }
[[ $run_fithist -eq 1 || $# == 0 ]] && { g++ pdana_fithist.cc $(root-config --libs --cflags) -I"../includes/" -g -o pdana_fithist.exe || exit 1 ; }

[[ $run_savehist -eq 1 ]] && { ./pdana_savehist.exe $input $tag $ptmin $ptmax $ymax $centmin $centmax ; }
input_fithist=rootfiles/${tag}_${tagki}/savehist.root
set -x
[[ $run_fithist -eq 1 ]] && { ./pdana_fithist.exe $input_fithist $tag ; }
set +x

[[ -f pdana_savehist.exe ]] && rm pdana_savehist.exe
[[ -f pdana_fithist.exe ]] && rm pdana_fithist.exe

