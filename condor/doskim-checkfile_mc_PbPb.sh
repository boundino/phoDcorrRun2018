#!/bin/bash

if [[ $0 != *.sh ]]
then
    echo -e "\e[31;1merror:\e[0m use \e[32;1m./script.sh\e[0m instead of \e[32;1msource script.sh\e[0m"
    return 1
fi

#
MAXFILENO=10000

#
movetosubmit=${1:-0}
runjobs=${2:-0}

# INPUTDIR="/mnt/hadoop/cms/store/user/wangj/QCDPhoton_pThat-120_Filter30GeV_TuneCP5_HydjetDrumMB_5p02TeV_Pythia8/crab_Dpho_20200924_QCDPhoton_pThat_120_Filter30GeV_TuneCP5_HydjetDrumMB_5p02TeV_Pythia8_trk1Dpt2/200926_030512/0000/"
# OUTPUTSUBDIR="phodmva_20200926_crab_Dpho_20200924_QCDPhoton_pThat_120_Filter30GeV_TuneCP5_HydjetDrumMB_trk1Dpt2"

# INPUTDIR="/mnt/hadoop/cms/store/user/wangj/QCDPhoton_pThat-80_Filter30GeV_TuneCP5_HydjetDrumMB_5p02TeV_Pythia8/crab_Dpho_20200924_QCDPhoton_pThat_80_Filter30GeV_TuneCP5_HydjetDrumMB_5p02TeV_Pythia8_trk1Dpt2/200926_022944/0000/" # 0001
# OUTPUTSUBDIR="phodmva_20200926_crab_Dpho_20200924_QCDPhoton_pThat_80_Filter30GeV_TuneCP5_HydjetDrumMB_trk1Dpt2"

# INPUTDIR="/mnt/hadoop/cms/store/user/wangj/QCDPhoton_pThat-50_Filter30GeV_TuneCP5_HydjetDrumMB_5p02TeV_Pythia8/crab_Dpho_20200924_QCDPhoton_pThat_50_Filter30GeV_TuneCP5_HydjetDrumMB_5p02TeV_Pythia8_trk1Dpt2/200926_022456/0000/" # 0001
# OUTPUTSUBDIR="phodmva_20200926_crab_Dpho_20200924_QCDPhoton_pThat_50_Filter30GeV_TuneCP5_HydjetDrumMB_trk1Dpt2"

# INPUTDIR="/mnt/hadoop/cms/store/user/wangj/QCDPhoton_pThat-30_Filter30GeV_TuneCP5_HydjetDrumMB_5p02TeV_Pythia8/crab_Dpho_20200924_QCDPhoton_pThat_30_Filter30GeV_TuneCP5_HydjetDrumMB_5p02TeV_Pythia8_trk1Dpt2/200926_021806/0000/" # 0001
# OUTPUTSUBDIR="phodmva_20200926_crab_Dpho_20200924_QCDPhoton_pThat_30_Filter30GeV_TuneCP5_HydjetDrumMB_trk1Dpt2"

INPUTDIR="/mnt/hadoop/cms/store/user/wangj/QCDPhoton_pThat-15_Filter30GeV_TuneCP5_HydjetDrumMB_5p02TeV_Pythia8/crab_Dpho_20200924_QCDPhoton_pThat_15_Filter30GeV_TuneCP5_HydjetDrumMB_5p02TeV_Pythia8_trk1Dpt2/200926_010834/0000/" # 0001
OUTPUTSUBDIR="phodmva_20200926_crab_Dpho_20200924_QCDPhoton_pThat_15_Filter30GeV_TuneCP5_HydjetDrumMB_trk1Dpt2"

OUTPUTPRIDIR="/mnt/hadoop/cms/store/user/jwang/DntupleRun2018condor/"

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
        ./skim-condor-checkfile.sh $INPUTDIR $OUTPUTDIR $MAXFILENO $LOGDIR
    else
        echo -e "\e[31;1merror:\e[0m submit jobs on \e[32;1msubmit.mit.edu\e[0m."
    fi
fi


# INPUTDIR="/mnt/hadoop/cms/store/user/wangj/PrmtD0_TuneCP5_HydjetDrumMB_5p02TeV_pythia8/crab_Dpho_20200924_PrmtD0_TuneCP5_HydjetDrumMB_5p02TeV_pythia8_trk1Dpt2/200925_013110/0000/" # 0002
# OUTPUTSUBDIR="phodmva_20200925_crab_Dpho_20200924_PrmtD0_TuneCP5_HydjetDrumMB_5p02TeV_pythia8_trk1Dpt2"
