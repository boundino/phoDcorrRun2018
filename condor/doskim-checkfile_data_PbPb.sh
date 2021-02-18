#!/bin/bash

if [[ $0 != *.sh ]]
then
    echo -e "\e[31;1merror:\e[0m use \e[32;1m./script.sh\e[0m instead of \e[32;1msource script.sh\e[0m"
    return 1
fi

#
MAXFILENO=100000
ishi=1
evtfilt=1
hltfilt=0
# mvafilt: [1] mva [2] cutbase [3] gen-matched
mvafilt=1
#
movetosubmit=${1:-0}
runjobs=${2:-0}

CONDITION='trk1Dpt2_phoEt19_rm0size'
PRIMARY='phodmva_20210201_Dpho_20210120'

INPUTS=(
    '/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIMinimumBias0/crab_Dpho_20210120_HIMinimumBias0_04Apr2019_trk1Dpt2_part_326381-326856/210123_005036/000*/'
    '/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIMinimumBias0/crab_Dpho_20210120_HIMinimumBias0_04Apr2019_trk1Dpt2_part_326859-326897/210123_005332/000*/'
    '/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIMinimumBias0/crab_Dpho_20210120_HIMinimumBias0_04Apr2019_trk1Dpt2_part_326941-327174/210123_005728/000*/'
    '/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIMinimumBias0/crab_Dpho_20210120_HIMinimumBias0_04Apr2019_trk1Dpt2_part_327211-327462/210123_005845/000*/'
    '/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIMinimumBias0/crab_Dpho_20210120_HIMinimumBias0_04Apr2019_trk1Dpt2_part_327464-327564/210123_010025/000*/'

    '/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIMinimumBias1/crab_Dpho_20210120_HIMinimumBias1_04Apr2019_trk1Dpt2_part_326381-326855/210123_010355/000*/'
    '/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIMinimumBias1/crab_Dpho_20210120_HIMinimumBias1_04Apr2019_trk1Dpt2_part_326856-326897/210123_011031/000*/'
    '/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIMinimumBias1/crab_Dpho_20210120_HIMinimumBias1_04Apr2019_trk1Dpt2_part_326941-327174/210123_011138/000*/'
    '/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIMinimumBias1/crab_Dpho_20210120_HIMinimumBias1_04Apr2019_trk1Dpt2_part_327211-327462/210123_011250/000*/'
    '/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIMinimumBias1/crab_Dpho_20210120_HIMinimumBias1_04Apr2019_trk1Dpt2_part_327464-327564/210123_011341/000*/'

    '/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIMinimumBias2/crab_Dpho_20210120_HIMinimumBias2_04Apr2019_trk1Dpt2_part_326381-326635/210123_003525/000*/'
    '/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIMinimumBias2/crab_Dpho_20210120_HIMinimumBias2_04Apr2019_trk1Dpt2_part_326636-326897/210123_004013/000*/'
    '/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIMinimumBias2/crab_Dpho_20210120_HIMinimumBias2_04Apr2019_trk1Dpt2_part_326941-327174/210123_004345/000*/'
    '/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIMinimumBias2/crab_Dpho_20210120_HIMinimumBias2_04Apr2019_trk1Dpt2_part_327211-327462/210123_004451/000*/'
    '/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIMinimumBias2/crab_Dpho_20210120_HIMinimumBias2_04Apr2019_trk1Dpt2_part_327464-327564/210123_004542/000*/'

    '/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIMinimumBias3/crab_Dpho_20210120_HIMinimumBias3_04Apr2019_trk1Dpt2_part_326381-326791/210123_011945/000*/'
    # '/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIMinimumBias3/'
    '/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIMinimumBias3/crab_Dpho_20210120_HIMinimumBias3_04Apr2019_trk1Dpt2_part_326941-327174/210123_012349/000*/'
    '/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIMinimumBias3/crab_Dpho_20210120_HIMinimumBias3_04Apr2019_trk1Dpt2_part_327211-327462/210123_012439/000*/'
    '/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIMinimumBias3/crab_Dpho_20210120_HIMinimumBias3_04Apr2019_trk1Dpt2_part_327464-327564/210123_012523/000*/'

    '/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIMinimumBias5/crab_Dpho_20210120_HIMinimumBias5_04Apr2019_trk1Dpt2_part_326381-326791/210123_012808/000*/'
    '/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIMinimumBias5/crab_Dpho_20210120_HIMinimumBias5_04Apr2019_trk1Dpt2_part_326815-326897/210123_013109/000*/'
    '/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIMinimumBias5/crab_Dpho_20210120_HIMinimumBias5_04Apr2019_trk1Dpt2_part_326941-327174/210123_013231/000*/'
    '/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIMinimumBias5/crab_Dpho_20210120_HIMinimumBias5_04Apr2019_trk1Dpt2_part_327211-327462/210123_013749/000*/'
    '/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIMinimumBias5/crab_Dpho_20210120_HIMinimumBias5_04Apr2019_trk1Dpt2_part_327464-327564/210123_013916/000*/'

    '/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIMinimumBias6/crab_Dpho_20210120_HIMinimumBias6_04Apr2019_trk1Dpt2_part_326381-326791/210123_040633/000*/'
    # ''
    '/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIMinimumBias6/crab_Dpho_20210120_HIMinimumBias6_04Apr2019_trk1Dpt2_part_326941-327174/210123_040939/000*/'
    '/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIMinimumBias6/crab_Dpho_20210120_HIMinimumBias6_04Apr2019_trk1Dpt2_part_327211-327462/210123_041026/000*/'
    # ''

    '/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIMinimumBias7/crab_Dpho_20210120_HIMinimumBias7_04Apr2019_trk1Dpt2_part_326381-326791/210123_042029/000*/'
    # ''
    # ''
    '/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIMinimumBias7/crab_Dpho_20210120_HIMinimumBias7_04Apr2019_trk1Dpt2_part_327211-327462/210123_042219/000*/'
    # ''

    '/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIMinimumBias9/crab_Dpho_20210120_HIMinimumBias9_04Apr2019_trk1Dpt2_part_326381-326791/210123_044351/000*/'
    # ''
    '/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIMinimumBias9/crab_Dpho_20210120_HIMinimumBias9_04Apr2019_trk1Dpt2_part_326941-327174/210123_044504/000*/'
    # ''
    # ''

    # ''
    # ''
    '/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIMinimumBias10/crab_Dpho_20210120_HIMinimumBias10_04Apr2019_trk1Dpt2_part_326941-327174/210123_044813/000*/'
    # ''
    # ''
)

WORKDIR="/work/$USER/phodmva/"
if [[ ! -d "$WORKDIR" ]]
then
    mkdir -p $WORKDIR
fi

for INPUTDIR in "${INPUTS[@]}"
do
    echo -e "\e[32;1m$INPUTDIR\e[0m"
    PD=${INPUTDIR##'/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/'} ; PD=${PD%%/*} ; RUN=${INPUTDIR##*part_} ; RUN=${RUN%%/*} ;
    OUTPUTSUBDIR="${PRIMARY}_${PD}_${RUN}_${CONDITION}"
    OUTPUTPRIDIR="/mnt/T2_US_MIT/hadoop/cms/store/user/jwang/DntupleRun2018condor/"

    ##
    OUTPUTDIR="${OUTPUTPRIDIR}/${OUTPUTSUBDIR}"
    LOGDIR="logs/log_${OUTPUTSUBDIR}"

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

# INPUTDIR="/mnt/hadoop/cms/store/user/wangj/HIHardProbes/crab_Dpho_20200921_HIHardProbes_04Apr2019_HIGEDPhoton40_trk1Dpt2/200921_210311/0000/" # 0000
