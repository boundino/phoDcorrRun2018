#!/bin/bash

var="dphi"
INPUTS=(
    # Input/unfold_pp_jtpt80_jteta1.6_pt6-20_y1.2_${var}.root
    # Input/unfold_pp_jtpt80_jteta1.6_pt20-30_y1.2_${var}.root            
    # Input/unfold_pp_jtpt80_jteta1.6_pt30-50_y1.2_${var}.root
    # Input/unfold_pp_jtpt80_jteta1.6_pt6-50_y1.2_${var}.root
    # Input/unfold_PbPb_jtpt80_jteta1.6_pt6-20_y1.2_cent090_${var}.root   
    Input/unfold_PbPb_jtpt80_jteta1.6_pt20-30_y1.2_cent090_${var}.root  
    # Input/unfold_PbPb_jtpt80_jteta1.6_pt30-50_y1.2_cent090_${var}.root  
    # Input/unfold_PbPb_jtpt80_jteta1.6_pt6-50_y1.2_cent010_${var}.root   
    # Input/unfold_PbPb_jtpt80_jteta1.6_pt6-50_y1.2_cent3050_${var}.root  
    # Input/unfold_PbPb_jtpt80_jteta1.6_pt6-50_y1.2_cent1030_${var}.root  
)

RUN_GENHIST=${1:-0}
RUN_UNFOLD=${2:-0}

[[ $# -eq 0 || $RUN_GENHIST -eq 1 ]] && { g++ toymc_genhist.cc $(root-config --libs --cflags) -I"../../includes/" -g -o toymc_genhist.exe || exit 2 ; }
[[ $# -eq 0 || $RUN_UNFOLD -eq 1 ]] && { g++ toymc_unfold.cc $(root-config --libs --cflags) -L/raid5/data/wangj/home/RooUnfold/build -lRooUnfold -I/raid5/data/wangj/home/RooUnfold/build -I"../../includes/" -g -o toymc_unfold.exe || exit 2 ; }

for input in "${INPUTS[@]}"
do
    echo $input
    [[ x$input == x ]] && exit 1 ;

    tag=${input##"Input/unfold_"}
    tag=${tag%%.root}
    echo $tag

    [[ $RUN_GENHIST -eq 1 ]] && ./toymc_genhist.exe $input "toymc_"$tag
    [[ $RUN_UNFOLD -eq 1 ]] && ./toymc_unfold.exe "ToyMC/toymc_"$tag".root" "toyunfolded_"$tag
done

rm toymc_genhist.exe
rm toymc_unfold.exe
