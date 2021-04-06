#!/bin/bash

if [[ $0 != *.sh ]]
then
    echo -e "\e[31;1merror:\e[0m use \e[32;1m./script.sh\e[0m instead of \e[32;1msource script.sh\e[0m"
    return 1
fi

#
MAXFILENO=10000
ishi=1
evtfilt=0
hltfilt=0
mvafilt=1
# mvafilt: [1] mva [2] cutbase [3] gen-matched
ptcut=70
rmsizezero=0

#
dccfile="skim-Djet.cc"
#
movetosubmit=${1:-0}
runjobs=${2:-0}

SELTAG=("nosel" "mva" "cut" "gmt")
FILTTAG=('' '_rmevt')

CONDITION="jtpt${ptcut}${FILTTAG[rmsizezero]}"
PRIMARY="djt${SELTAG[mvafilt]}_20210322"

INPUTS=(
    # "/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIHardProbes/crab_Dpho_20200921_HIHardProbes_04Apr2019_HIGEDPhoton40_trk1Dpt2/200921_210311/000*/"
    # "/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIHardProbes/crab_Djet_20210315_HIHardProbes_04Apr2019_PuAK4CaloJet80100Eta5p1_trk1Dpt2/210315_213900/000*/"
    "/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIMinimumBias0/crab_Djet_20210315_HIMinimumBias0_04Apr2019_trk1Dpt2_part_326381-326791/210323_053938/000*/"
    # "/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIMinimumBias0/crab_Djet_20210315_HIMinimumBias0_04Apr2019_trk1Dpt2_part_326815-326897/210323_055750/000*/"
    # "/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIMinimumBias0/crab_Djet_20210315_HIMinimumBias0_04Apr2019_trk1Dpt2_part_326941-327174/210323_055833/000*/"
)

WORKDIR="/work/$USER/phodmva/"
if [[ ! -d "$WORKDIR" ]]
then
    mkdir -p $WORKDIR
fi

for INPUTDIR in "${INPUTS[@]}"
do
    echo -e "\e[32;1m$INPUTDIR\e[0m"
    REQUESTNAME=${INPUTDIR##*crab_} ; REQUESTNAME=${REQUESTNAME%%/*} ;
    OUTPUTSUBDIR="${PRIMARY}_${REQUESTNAME}_${CONDITION}"
    OUTPUTPRIDIR="/mnt/T2_US_MIT/hadoop/cms/store/user/jwang/DntupleRun2018condor/"

    ##
    OUTPUTDIR="${OUTPUTPRIDIR}/${OUTPUTSUBDIR}"
    LOGDIR="logs/log_${OUTPUTSUBDIR}"

    echo "$OUTPUTDIR"
    ##

    if [ "$runjobs" -eq 1 ]
    then 
        if [[ $(hostname) == "submit.mit.edu" ]]
        then
            set -x
            ./skim-condor-checkfile.sh "$INPUTDIR" $OUTPUTDIR $MAXFILENO $LOGDIR $ishi $evtfilt $hltfilt $mvafilt $ptcut $rmsizezero
            set +x
        else
            echo -e "\e[31;1merror:\e[0m submit jobs on \e[32;1msubmit.mit.edu\e[0m."
        fi
    fi

done

if [ "$movetosubmit" -eq 1 ]
then
    if [[ $(hostname) == "submit-hi2.mit.edu" || $(hostname) == "submit-hi1.mit.edu" ]]
    then
        cd ../skim
        g++ $dccfile $(root-config --cflags --libs) -I"../includes/" -lTMVA -lXMLIO -Werror -Wall -O2 -o skim.exe || exit 1
        cd ../condor

        mv -v ../skim/skim.exe $WORKDIR/
        cp -v $0 $WORKDIR/
        cp -v skim-skimd-checkfile.sh $WORKDIR/
        cp -v skim-condor-checkfile.sh $WORKDIR/
        cp -v ../skim/mva.tgz $WORKDIR/
    else
        echo -e "\e[31;1merror:\e[0m compile macros on \e[32;1msubmit-hiX.mit.edu\e[0m or \e[32;1msubmit.mit.edu\e[0m."
    fi
fi
