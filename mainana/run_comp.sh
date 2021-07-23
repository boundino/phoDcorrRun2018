#!/bin/bash

tag="widebin"

input_pp=rootfiles/ana_finebin_trig_pp_data_phopt40_phoeta1.442_pt4-10_y1.2/calchist.root
input_PbPb=rootfiles/ana_${tag}_trig_PbPb_data_phopt40_phoeta1.442_pt4-10_y1.2_cent090/calchist.root
input_emix=rootfiles/ana_${tag}_emix_PbPb_data_phopt40_phoeta1.442_pt4-10_y1.2_cent090/calchist.root
input_MC_pp=mcrootfiles/mc_pythia_pp_MC_phopt40_phoeta1.442_pt4-10_y1.2/drawhist.root
input_MC_PbPb=mcrootfiles/mc_embed_PbPb_MC_phopt40_phoeta1.442_pt4-10_y1.2_cent090/drawhist.root
#
tag="comp_${tag}"

##
run_comphist=${1:-0}

ls -l $input_pp || exit 1
ls -l $input_PbPb || exit 1
ls -l $input_emix || exit 1
ls -l $input_MC_pp || exit 1
ls -l $input_MC_PbPb || exit 1

##
[[ $run_comphist -eq 1 || $# == 0 ]] && { g++ comp_drawhist.cc $(root-config --libs --cflags) -I"../includes/" -g -o comp_drawhist.exe || exit 1 ; }

[[ $run_comphist -eq 1 ]] && { ./comp_drawhist.exe $input_pp $input_PbPb $input_emix $input_MC_pp $input_MC_PbPb $tag ; }

rm comp_drawhist.exe > /dev/null 2>&1
