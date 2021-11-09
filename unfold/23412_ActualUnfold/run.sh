#!/bin/bash

closure=0
var="dphi"
INPUTS=(
    Input/unfold_pp_jtpt80_jteta1.6_pt6-20_y1.2_${var}.root
    Input/unfold_pp_jtpt80_jteta1.6_pt20-30_y1.2_${var}.root            
    Input/unfold_pp_jtpt80_jteta1.6_pt30-50_y1.2_${var}.root
    Input/unfold_pp_jtpt80_jteta1.6_pt6-50_y1.2_${var}.root
    Input/unfold_PbPb_jtpt80_jteta1.6_pt6-20_y1.2_cent090_${var}.root   
    Input/unfold_PbPb_jtpt80_jteta1.6_pt20-30_y1.2_cent090_${var}.root  
    Input/unfold_PbPb_jtpt80_jteta1.6_pt30-50_y1.2_cent090_${var}.root  
    Input/unfold_PbPb_jtpt80_jteta1.6_pt6-50_y1.2_cent010_${var}.root   
    Input/unfold_PbPb_jtpt80_jteta1.6_pt6-50_y1.2_cent3050_${var}.root  
    Input/unfold_PbPb_jtpt80_jteta1.6_pt6-50_y1.2_cent1030_${var}.root  
)

RUN_UNFOLD=${1:-0}
RUN_DRAW=${2:-0}

./compile.sh
g++ draw.cpp $(root-config --libs --cflags) -I"../../includes/" -g -o draw.exe || exit 2 ;

for input in "${INPUTS[@]}"
do
    echo $input
    [[ x$input == x ]] && exit 1 ;

    tag=${input##"Input/unfold_"}
    tag=${tag%%.root}
    echo $tag

    edition="Unfolded" ; InputName="--InputName HDataReco" ;
    if [[ $closure -eq 1 ]]
    then
        edition="Closure"
        InputName="--InputName HMCMatchedGenptRecophi"
    elif [[ $closure -eq 2 ]]
    then
        edition="MCPrior"
        InputName="--InputName HDataReco --MCPrior true"
    fi

    [[ $RUN_UNFOLD -eq 1 ]] && ./Execute --Input $input --Output "Output/${edition}_"$tag".root" $InputName
    # ./ExecutePlot --Input "Output/${edition}_"$tag".root" --Original $input --Output "plots/${edition}_"${tag}".pdf"
    [[ $RUN_DRAW -eq 1 ]] && ./draw.exe "Output/${edition}_"$tag".root" $input "${edition}_"$tag
done
