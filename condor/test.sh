#!/bin/bash

i=/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/QCDPhoton_pThat-30_Filter30GeV_TuneCP5_HydjetDrumMB_5p02TeV_Pythia8/crab_Dpho_20200924_QCDPhoton_pThat_30_Filter30GeV_TuneCP5_HydjetDrumMB_5p02TeV_Pythia8_trk1Dpt2/200926_021806/0000/HiForestAOD_100.root

file=${i##*/}
inputdir=${i%%$file}
infn=${file%%.*}
remove=${i/\/T2_US_MIT/}

echo $file
echo $inputdir
echo $infn
echo $remove
echo "-------------------------------------------------------"


CONDITION='trk1Dpt2_phoEt19_rm0size'

INPUTDIR='/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/HIMinimumBias0/crab_Dpho_20210120_HIMinimumBias0_04Apr2019_trk1Dpt2_part_326381-326856/210123_005036/000*'

PD=${INPUTDIR##'/mnt/T2_US_MIT/hadoop/cms/store/user/wangj/'}
PD=${PD%%/*}
RUN=${INPUTDIR##*part_}
RUN=${RUN%%/*}
OUTPUTSUBDIR="phodmva_20210126_Dpho_20210120_${PD}_${RUN}_${CONDITION}"

echo $OUTPUTSUBDIR
