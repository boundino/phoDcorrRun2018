#!/bin/bash

ikine=(0 1)
# ikine
# Dptmin Dptmax Dymax centmin centmax jetptmin jetetamax
config=(
  "6 30 1.2 0 90 80 1.6"  # 0 default
  "30 999 1.2 0 90 80 1.6"  # 1 default
)

##
syst="Djet"
name="djcomp"

run_comphist=${1:-0}

##
g++ getfname.cc -I"../includes/" $(root-config --libs --cflags) -g -o getfname.exe || { rm *.exe 2> /dev/null ; exit 1 ; }
[[ $run_comphist -eq 1 || $# == 0 ]] && { g++ ${name}_drawhist.cc $(root-config --libs --cflags) -I"../includes/" -g -o ${name}_drawhist.exe || exit 1 ; }

for i in ${ikine[@]}
do
    # input_pp=$(./getfname.exe $ishi ${config[i]} $ismc "$syst")
    input_pp=rootfiles/ana_finebin_trig_$(./getfname.exe 0 ${config[i]} 0 "$syst")/calchist.root
    input_PbPb=rootfiles/ana_finebin_trig_$(./getfname.exe 1 ${config[i]} 0 "$syst")/calchist.root
    input_emix=rootfiles/ana_finebin_emix_$(./getfname.exe 1 ${config[i]} 0 "$syst")/calchist.root
    input_MC_pp=mcrootfiles/mc_pythia_$(./getfname.exe 0 ${config[i]} 1 "$syst")/drawhist.root
    input_MC_PbPb=mcrootfiles/mc_embed_$(./getfname.exe 1 ${config[i]} 1 "$syst")/drawhist.root
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

