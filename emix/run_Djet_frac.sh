#!/bin/bash

inputs=(
    /raid5/data/wangj/DntupleRun2018/djtmva_20211111_Djet_20211110_HIMinimumBias1_MiniAODv1_trk1Dpt4_jtpt50_rmevt.root,/raid5/data/wangj/DntupleRun2018/djtmva_20211111_Djet_20211110_HIMinimumBias2_MiniAODv1_trk1Dpt4_jtpt60.root
    /raid5/data/wangj/DntupleRun2018/djtmva_20211111_Djet_20211110_HIMinimumBias2_MiniAODv1_trk1Dpt4_jtpt50_rmevt.root,/raid5/data/wangj/DntupleRun2018/djtmva_20211111_Djet_20211110_HIMinimumBias3_MiniAODv1_trk1Dpt4_jtpt60.root
    /raid5/data/wangj/DntupleRun2018/djtmva_20211111_Djet_20211110_HIMinimumBias3_MiniAODv1_trk1Dpt4_jtpt50_rmevt.root,/raid5/data/wangj/DntupleRun2018/djtmva_20211111_Djet_20211110_HIMinimumBias4_MiniAODv1_trk1Dpt4_jtpt60.root
    /raid5/data/wangj/DntupleRun2018/djtmva_20211111_Djet_20211110_HIMinimumBias4_MiniAODv1_trk1Dpt4_jtpt50_rmevt.root,/raid5/data/wangj/DntupleRun2018/djtmva_20211111_Djet_20211110_HIMinimumBias5_MiniAODv1_trk1Dpt4_jtpt60.root
    /raid5/data/wangj/DntupleRun2018/djtmva_20211111_Djet_20211110_HIMinimumBias5_MiniAODv1_trk1Dpt4_jtpt50_rmevt.root,/raid5/data/wangj/DntupleRun2018/djtmva_20211111_Djet_20211110_HIMinimumBias1_MiniAODv1_trk1Dpt4_jtpt60.root
)
nmix=1
nfrac=10

unique=$(date +%s)
g++ mixevents_Djet_frac.cc $(root-config --cflags --libs) -I"../includes/" -g -o mixevents_Djet_frac_${unique}.exe || exit 1

RUN_MIX=${1:-0}

for input in ${inputs[@]}
do
    IFS=',' ; iss=($input) ; unset IFS ;
    input_trig=${iss[0]}
    input_mb=${iss[1]}
    j=0
    while [[ $j -lt $nfrac ]]
    do
        fraction=$nfrac
        ifrac=$j
        # fraction=${iss[2]}
        # ifrac=${iss[3]}
        
        output=${input_trig%%.root}_NMIX${nmix}_eta2_part${ifrac}-${fraction}.root

        echo $input_trig
        echo $input_mb
        echo $output

        [[ $RUN_MIX -eq 1 ]] && ./mixevents_Djet_frac_${unique}.exe $input_trig $input_mb $output $nmix $fraction $ifrac &

        j=$((j+1))
    done
    wait
done

rm mixevents_Djet_frac_${unique}.exe
