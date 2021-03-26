#!/bin/bash

tag="widebin"

ik=({0..5})
kinematics=(
    5-100,0.05 
    5-30,0.05 
    4-100,0.12 
    4-30,0.12 
    4-10,0.16
    2-10,0.38
)


#
run_projhist=${1:-0}

##
[[ $run_projhist -eq 1 || $# == 0 ]] && { g++ proj_drawhist.cc $(root-config --libs --cflags) -I"../includes/" -g -o proj_drawhist.exe || exit 1 ; }

for i in ${ik[@]}
do
    kk=${kinematics[i]}
    IFS=',' ; vk=($kk) ; unset IFS
    KINE=${vk[0]}
    mtdgain=${vk[1]}
    input_pp=rootfiles/ana_finebin_trig_pp_data_phopt40_phoeta1.442_pt${KINE}_y1.2/calchist.root
    input_PbPb=rootfiles/ana_${tag}_trig_PbPb_data_phopt40_phoeta1.442_pt${KINE}_y1.2_cent090/calchist.root
    input_emix=rootfiles/ana_${tag}_emix_PbPb_data_phopt40_phoeta1.442_pt${KINE}_y1.2_cent090/calchist.root

    echo "MTD significance gain $mtdgain"
    ls -l $input_pp || exit 1
    ls -l $input_PbPb || exit 1
    ls -l $input_emix || exit 1

    [[ $run_projhist -eq 1 ]] && { ./proj_drawhist.exe $input_pp $input_PbPb $input_emix $tag $mtdgain; }
done

rm proj_drawhist.exe > /dev/null 2>&1
