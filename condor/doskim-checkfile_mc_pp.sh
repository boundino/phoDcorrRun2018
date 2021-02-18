#!/bin/bash

if [[ $0 != *.sh ]]
then
    echo -e "\e[31;1merror:\e[0m use \e[32;1m./script.sh\e[0m instead of \e[32;1msource script.sh\e[0m"
    return 1
fi

#
MAXFILENO=1000
ishi=0
evtfilt=0
hltfilt=0
mvafilt=2
# mvafilt: [1] mva [2] cutbase [3] gen-matched

#
movetosubmit=${1:-0}
runjobs=${2:-0}

SELTAG=("nosel" "mva" "cut" "gmatch")

CONDITION='trk1Dpt2_phoEt30'
PRIMARY="phod${SELTAG[mvafilt]}_20210201_Dpho"

INPUTS=(
    "/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/Pythia8_DzeroToKPi_prompt_Dpt0p0_Pthat0_TuneCP5_5020GeV/crab_Dpho_20201104_DzeroToKPi_prompt_Dpt0p0_Pthat0_wangj_MC_20200219_trk1Dpt2/210217_184251/000*/"
    # "/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/Pythia8_DzeroToKPi_inclusive_Dpt0p0_Pthat0_TuneCP5_5020GeV/crab_Dpho_20201104_DzeroToKPi_inclusive_Dpt0p0_Pthat0_wangj_MC_20200219_trk1Dpt2/210217_213729/000*/"
    # "/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/QCDPhoton_pThat-170_TuneCP5_5p02TeV_pythia8/crab_Dpho_20201104_QCDPhoton_pThat_170_mc2017_realistic_forppRef5TeV_trk1Dpt2/201104_183531/000*/"
    # "/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/QCDPhoton_pThat-120_TuneCP5_5p02TeV_pythia8/crab_Dpho_20201104_QCDPhoton_pThat_120_mc2017_realistic_forppRef5TeV_trk1Dpt2/201104_183052/000*/"
    # "/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/QCDPhoton_pThat-80_TuneCP5_5p02TeV_pythia8/crab_Dpho_20201104_QCDPhoton_pThat_80_mc2017_realistic_forppRef5TeV_trk1Dpt2/201104_183019/000*/"
    # "/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/QCDPhoton_pThat-50_TuneCP5_5p02TeV_pythia8/crab_Dpho_20201104_QCDPhoton_pThat_50_mc2017_realistic_forppRef5TeV_trk1Dpt2/201104_182724/000*/"
    # "/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/QCDPhoton_pThat-30_TuneCP5_5p02TeV_pythia8/crab_Dpho_20201104_QCDPhoton_pThat_30_mc2017_realistic_forppRef5TeV_trk1Dpt2/201104_180415/000*/"
)

WORKDIR="/work/$USER/phodmva/"
if [[ ! -d "$WORKDIR" ]]
then
    mkdir -p $WORKDIR
fi

for INPUTDIR in "${INPUTS[@]}"
do
    echo -e "\e[32;1m$INPUTDIR\e[0m"
    PD=${INPUTDIR##'/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/'} ; PD=${PD%%/*} ; DAT=${INPUTDIR##*crab_Dpho_} ; DAT=${DAT%%_*} ;
    OUTPUTSUBDIR="${PRIMARY}_${DAT}_${PD}_${CONDITION}"
    OUTPUTPRIDIR="/mnt/T2_US_MIT/hadoop/cms/store/user/jwang/DntupleRun2017condor/"

    ##
    OUTPUTDIR="${OUTPUTPRIDIR}/${OUTPUTSUBDIR}"
    LOGDIR="logs/log_${OUTPUTSUBDIR}"

    echo "$OUTPUTDIR"
    ##

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
            ./skim-condor-checkfile.sh "$INPUTDIR" $OUTPUTDIR $MAXFILENO $LOGDIR $ishi $evtfilt $hltfilt $mvafilt
        else
            echo -e "\e[31;1merror:\e[0m submit jobs on \e[32;1msubmit.mit.edu\e[0m."
        fi
    fi

done

# INPUTDIR="/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/PrmtD0_TuneCP5_HydjetDrumMB_5p02TeV_pythia8/crab_Dpho_20200924_PrmtD0_TuneCP5_HydjetDrumMB_5p02TeV_pythia8_trk1Dpt2/200925_013110/0000/" # 0002
# OUTPUTSUBDIR="phodmva_20200925_Dpho_20200924_PrmtD0_TuneCP5_HydjetDrumMB_5p02TeV_pythia8_trk1Dpt2"
