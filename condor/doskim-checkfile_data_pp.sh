#!/bin/bash

if [[ $0 != *.sh ]]
then
    echo -e "\e[31;1merror:\e[0m use \e[32;1m./script.sh\e[0m instead of \e[32;1msource script.sh\e[0m"
    return 1
fi

#
MAXFILENO=10000
ishi=0
evtfilt=0
hltfilt=1
mvafilt=1
#
movetosubmit=${1:-0}
runjobs=${2:-0}

INPUTDIR="/mnt/hadoop/cms/store/user/wangj/HighEGJet/crab_Dpho_20201002_HighEGJet_Run2017G_17Nov2017_HIPhoton_HoverELoose_trk1Dpt2_vfl2/201002_173802/0003/" # 0003
OUTPUTPRIDIR="/mnt/hadoop/cms/store/user/jwang/DntupleRun2017condor/"
OUTPUTSUBDIR="phodcut_20201002_Dpho_20201002_HighEGJet_17Nov2017_HIPhoton_HoverELoose_trk1Dpt2_vfl2"

WORKDIR="/work/$USER/phodmva/"

##
OUTPUTDIR="${OUTPUTPRIDIR}/${OUTPUTSUBDIR}"
LOGDIR="logs/log_${OUTPUTSUBDIR}"

##

if [[ ! -d "$WORKDIR" ]]
then
    mkdir -p $WORKDIR
fi

if [ "$movetosubmit" -eq 1 ]
then
    if [[ $(hostname) == "submit-hi2.mit.edu" || $(hostname) == "submit.mit.edu" || $(hostname) == "submit-hi1.mit.edu" ]]
    then
        cd ../skim
        g++ skim.cc $(root-config --cflags --libs) -I"../includes/" -lTMVA -lXMLIO -Werror -Wall -O2 -o skim.exe || exit 1
        cd ../condor

        mv -v ../skim/skim.exe $WORKDIR/
        cp -v $0 $WORKDIR/
        cp -v skim-phod-checkfile.sh $WORKDIR/
        cp -v skim-condor-checkfile.sh $WORKDIR/
        cp -v ../skim/mva.tgz $WORKDIR/
    else
        echo -e "\e[31;1merror:\e[0m compile macros on \e[32;1msubmit-hiX.mit.edu\e[0m or \e[32;1msubmit.mit.edu\e[0m."
    fi
fi

if [ "$runjobs" -eq 1 ]
then 
    if [[ $(hostname) == "submit.mit.edu" ]]
    then
        ./skim-condor-checkfile.sh $INPUTDIR $OUTPUTDIR $MAXFILENO $LOGDIR $ishi $evtfilt $hltfilt $mvafilt
    else
        echo -e "\e[31;1merror:\e[0m submit jobs on \e[32;1msubmit.mit.edu\e[0m."
    fi
fi
