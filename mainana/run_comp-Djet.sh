#!/bin/bash

config=(
    # "6 30 1.2 0 90 80 1.6"
    # "30 999 1.2 0 90 80 1.6"
    "6 10 1.2 0 90 80 1.6"
    "10 20 1.2 0 90 80 1.6"
    "20 30 1.2 0 90 80 1.6"
    "30 50 1.2 0 90 80 1.6"
    "50 80 1.2 0 90 80 1.6"

    "6 10 2 0 90 80 2"
    "10 20 2 0 90 80 2"
    "20 30 2 0 90 80 2"
    "30 50 2 0 90 80 2"
    "50 80 2 0 90 80 2"
)

##
syst="Djet"
name="djcomp"

run_comphist=${1:-0}

##
g++ getfname.cc -I"../includes/" $(root-config --libs --cflags) -g -o getfname.exe || { rm *.exe 2> /dev/null ; exit 1 ; }
[[ $run_comphist -eq 1 || $# == 0 ]] && { g++ ${name}_drawhist.cc $(root-config --libs --cflags) -I"../includes/" -g -o ${name}_drawhist.exe || exit 1 ; }

for iconfig in "${config[@]}"
do
    # input_pp=$(./getfname.exe $ishi ${iconfig} $ismc "$syst")
    input_pp=rootfiles/ana_finebin_trig_$(./getfname.exe 0 ${iconfig} 0 "$syst")/calchist.root
    input_PbPb=rootfiles/ana_finebin_trig_$(./getfname.exe 1 ${iconfig} 0 "$syst")/calchist.root
    input_emix=rootfiles/ana_finebin_emix_$(./getfname.exe 1 ${iconfig} 0 "$syst")/calchist.root
    input_MC_pp=mcrootfiles/mc_pythia_$(./getfname.exe 0 ${iconfig} 1 "$syst")/drawhist.root
    input_MC_PbPb=mcrootfiles/mc_embed_$(./getfname.exe 1 ${iconfig} 1 "$syst")/drawhist.root
    #
    tag="comp_finebin"

    ##
    run_comphist=${1:-0}

    ls -l $input_pp
    ls -l $input_PbPb
    ls -l $input_emix
    ls -l $input_MC_pp
    ls -l $input_MC_PbPb

    [[ $run_comphist -eq 1 ]] && { ./${name}_drawhist.exe $tag $input_pp $input_PbPb $input_emix $input_MC_pp $input_MC_PbPb ; }

done

rm getfname.exe > /dev/null 2>&1
rm ${name}_drawhist.exe > /dev/null 2>&1

