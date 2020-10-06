#!/bin/bash

ikine=(1 2)

# rootfiles/prel_finebin_pp_phopt40_pt5-100_y1.2/calchist.root
config=(
    "'rootfiles/prel_finebin_pp_*_pt5-100_y1.2/calchist.root' phopt40,phopt50,phopt60,phopt80,phopt100 prel_finebin_pp_pt5-100_y1.2 phopt histe"
    "'rootfiles/prel_finebin_pp_phopt40_*_y1.2/calchist.root' pt2-100,pt2-5 prel_finebin_pp_phopt40_y1.2 pt histe"
    "'rootfiles/prel_finebin_PbPb_phopt40_*_y1.2_cent090/calchist.root' pt2-100,pt2-5 prel_finebin_PbPb_phopt40_y1.2_cent090 pt histe"
)

##
run_comphist=${1:-0}

##
[[ $run_comphist -eq 1 || $# == 0 ]] && { g++ comphist.cc $(root-config --libs --cflags) -I"../includes/" -g -o comphist.exe || exit 1 ; }

for i in ${ikine[@]}
do
    [[ $run_comphist -eq 1 ]] && { ./comphist.exe ${config[i]} ; }
done

rm comphist.exe > /dev/null 2>&1
