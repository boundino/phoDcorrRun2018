#!/bin/bash

##
trainlabel="_210730"
# -- is heavy ion or not
ishi=0 ; evtweights="1" # pthatweight*Ncoll for PbPb
# -- signal sample
inputs=/raid5/data/wangj/DntupleRun2017/djtgmt_20210629_Dfinder_20210629_Pythia8_DzeroToKPi_prompt_Dpt0p0_Pthat0_trk1Dpt2_v3_jtpt70.root ;
# -- background sample
inputb=/raid5/data/wangj/DntupleRun2017/djtnosel_20210629_Dfinder_20210629_HIZeroBias1_Run2017G-17Nov2017_trk1Dpt2_jtpt70_0000.root ; 

##
coll=("pp" "PbPb")
output=rootfiles/TMVA_${coll[ishi]}${trainlabel}

# -- event filter
cut="pBeamScrapingFilter && pPAprimaryVertexFilter"
# -- track kinematics
cut=$cut" && Dtrk1Pt > 1 && Dtrk2Pt > 1 && TMath::Abs(Dtrk1Eta) < 2.4 && TMath::Abs(Dtrk2Eta) < 2.4"
# -- track qualirty
cut=$cut" && Dtrk1highPurity && Dtrk2highPurity && TMath::Abs(Dtrk1PtErr/Dtrk1Pt) < 0.1 && TMath::Abs(Dtrk2PtErr/Dtrk2Pt) < 0.1"
# -- X prefilter
cut=$cut" && Dchi2cl > 0.05"
# -- tricky selection
cut=$cut" && TMath::Abs(DsvpvDistance_2D/DsvpvDisErr_2D)<60"

##
algo="BDT,BDTG,BDTF,CutsSA,LD"
# algo="BDT"

# stages="1,2,3,4,5,6,7,8,9,10,11" ; sequence=1 ; # see definition below #
stages="1,2,3,4,11" ; sequence=0 ; # see definition below #

## ===== do not change the lines below =====
varstrategy=("Single set" "Sequence")

cuts="${cut} && Dgen==23333 && DgencollisionId==0"
cutb="${cut} && TMath::Abs(Dmass-1.8649) > 0.07 && TMath::Abs(Dmass-1.8649) < 0.12" # sideband
# [[ $bkgstrategy == "sdb" ]] && cutb="${cut} && TMath::Abs(Dmass-1.8649) > 0.07 && TMath::Abs(Dmass-1.8649) < 0.12" # sideband
# [[ $bkgstrategy == "ssn" ]] && cutb="${cut} && TMath::Abs(Dmass-1.8649) < 0.045" # samesign

## ===== do not do not do not change the lines below =====
function catspace() { echo -e $(cat "$@" | sed  's/$/\\n/' | sed 's/ /\\a /g') ; }
IFS=','; allstages=($stages); unset IFS;
echo -e '
##########################
# Variables \e[1;32m(To be used)\e[0m #
##########################'
vv=0
catspace TMVAClassification.h | grep --color=no 'mytmva::tmvavar("' > tmpvarlist.list
while read -r line
do
    for ss in ${allstages[@]} ; do [[ $ss == $vv ]] && { echo -en "\e[1;32m" ; break ; } ; done ;
    echo -e $line | sed 's/mytmva::tmvavar//' | sed 's/\*\///' | sed 's/\a \a \/\*//' ; echo -ne "\e[0m" ;
    vv=$((vv+1))
done < tmpvarlist.list
rm tmpvarlist.list

##
echo -e "
###########################
# Training Configurations #
###########################

>>>>>> Variables training strategy
  >>>> \e[32m[${varstrategy[sequence]}]\e[0m

>>>>>> Event weights
  >>>> \e[32m[\"$evtweights\"]\e[0m

>>>>>> Algorithms
  >>>> \e[32m[$algo]\e[0m

>>>>>> Input files
  >>>> Signal:      \e[32m$inputs\e[0m
  >>>> Background:  \e[32m$inputb\e[0m

>>>>>> Selections
  >>>> Prefilters
    >> \e[32m\"$cut\"\e[0m
  >>>> Signal cut
    >> \e[32m\"${cuts##$cut}\"\e[0m
  >>>> Background cut
    >> \e[32m\"${cutb##$cut}\"\e[0m
"

##
[[ -d $output ]] && rm -r $output
# mkdir -p $outputmvadir
tmp=$(date +%y%m%d%H%M%S)

##
[[ $# -eq 0 ]] && echo "usage: ./run_TMVAClassification.sh [train] [draw curves] [create BDT tree]"
echo "Compiling .cc macros..."

echo -e "\e[35m==> (1/5) building TMVAClassification.C\e[0m"
g++ TMVAClassification.C $(root-config --libs --cflags) -lTMVA -lTMVAGui -I"../../includes/" -g -o TMVAClassification_${tmp}.exe || { exit 1 ; }
echo -e "\e[35m==> (2/5) building guivariables.C\e[0m"
g++ guivariables.C $(root-config --libs --cflags) -lTMVA -lTMVAGui -I"../../includes/" -g -o guivariables_${tmp}.exe             || { rm *_${tmp}.exe ; exit 1 ; }
echo -e "\e[35m==> (3/5) building guiefficiencies.C\e[0m"
g++ guiefficiencies.C $(root-config --libs --cflags) -lTMVA -lTMVAGui -I"../../includes/" -g -o guiefficiencies_${tmp}.exe       || { rm *_${tmp}.exe ; exit 1 ; }
echo -e "\e[35m==> (4/5) building guieffvar.C\e[0m"
g++ guieffvar.C $(root-config --libs --cflags) -lTMVA -lTMVAGui -I"../../includes/" -g -o guieffvar_${tmp}.exe                   || { rm *_${tmp}.exe ; exit 1 ; }
echo -e "\e[35m==> (5/5) building mvaprod.C\e[0m"
g++ mvaprod.C $(root-config --libs --cflags) -lTMVA -lXMLIO -lstdc++fs -I"../../includes/" -g -o mvaprod_${tmp}.exe              || { rm *_${tmp}.exe ; exit 1 ; }

[[ ${1:-0} -eq 1 ]] && {
    conf=
    echo -e "\e[2m==> Do you really want to run\e[0m \e[1mTMVAClassification.C\e[0m \e[2m(it might be very slow)?\e[0m [y/n]"
    read conf
    while [[ $conf != 'y' && $conf != 'n' ]] ; do { echo "warning: input [y/n]" ; read conf ; } ; done ;
    [[ $conf == 'n' ]] && { rm *_${tmp}.exe ; exit ; }
}

# train
stage=$stages
while [[ $stage == *,* ]]
do
    [[ ${1:-0} -eq 1 ]] && { ./TMVAClassification_${tmp}.exe $inputs $inputb "$cuts" "$cutb" "$evtweights" $output "$algo" "$stage"; } 
    [[ $sequence -eq 0 ]] && break;
    while [[ $stage != *, ]] ; do stage=${stage%%[0-9]} ; done ;
    stage=${stage%%,}
done

# draw curves
[[ ${2:-0} -eq 1 ]] && { 
    ./guivariables_${tmp}.exe $output "$algo" "$stages"
    ./guiefficiencies_${tmp}.exe $output "$algo" "$stages"
}
# draw curve vs. var
[[ ${2:-0} -eq 1 && $sequence -eq 1 ]] && ./guieffvar_${tmp}.exe $output "$algo" "$stages"

# produce mva values
# for ioutmva in ${outmvas[@]}
# {
#     inputm=${inputms[ioutmva]}
#     [[ ${3:-0} -eq 1 ]] && ./mvaprod_${tmp}.exe $inputm "Bfinder/ntmix" $output $outputmvadir "$algo" "${stages}"
# }

##
rm *_${tmp}.exe

