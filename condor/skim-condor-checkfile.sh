#!/bin/bash

if [[ $# -ne 8 ]]; then
    echo "usage: ./skim-condor-checkfile.sh [input dir] [output dir] [max jobs] [log dir] [ishi] [evtfilt] [hltfilt] [mvafilt]"
    exit 1
fi

DATASET=$1
DESTINATION=$2
MAXFILES=$3
LOGDIR=$4
ISHI=$5
EVTFILT=$6
HLTFILT=$7
MVAFILT=$8

PROXYFILE=$(ls /tmp/ -lt | grep $USER | grep -m 1 x509 | awk '{print $NF}')

tag="phod"

rm filelist.txt
# ls $DATASET | grep -v "/" | grep -v -e '^[[:space:]]*$' | awk '{print "" $0}' >> filelist.txt
ls -d $DATASET/* >> filelist.txt

DEST_CONDOR=${DESTINATION/\/T2_US_MIT/}
SRM_PREFIX="/mnt/hadoop/"
SRM_PATH=${DEST_CONDOR#${SRM_PREFIX}}

if [ ! -d $DESTINATION ]
then
    # gfal-mkdir -p srm://se01.cmsaf.mit.edu:8443/srm/v2/server?SFN=$DESTINATION
    LD_LIBRARY_PATH='' PYTHONPATH='' gfal-mkdir -p gsiftp://se01.cmsaf.mit.edu:2811/${SRM_PATH}
fi

mkdir -p $LOGDIR

counter=0
for i in `cat filelist.txt`
do
    if [ $counter -ge $MAXFILES ]
    then
        break
    fi
    inputname=${i/\/T2_US_MIT/}
    infn=${inputname##*/}
    infn=${infn%%.*} # no .root
    outputfile=${tag}_${infn}.root
    if [ ! -f ${DESTINATION}/${outputfile} ] && [ -f $i ]
    then
        if [ -s $i ]
        then
            echo -e "\033[38;5;242mSubmitting a job for output\033[0m ${DESTINATION}/${outputfile}"
                
                cat > skim-${tag}.condor <<EOF

Universe     = vanilla
Initialdir   = $PWD/
Notification = Error
Executable   = $PWD/skim-${tag}-checkfile.sh
Arguments    = $inputname $DEST_CONDOR ${outputfile} $ISHI $EVTFILT $HLTFILT $MVAFILT $PROXYFILE 
GetEnv       = True
Output       = $LOGDIR/log-${infn}.out
Error        = $LOGDIR/log-${infn}.err
Log          = $LOGDIR/log-${infn}.log
Rank         = Mips
+AccountingGroup = "group_cmshi.$(whoami)"
requirements = GLIDEIN_Site == "MIT_CampusFactory" && BOSCOGroup == "bosco_cmshi" && HAS_CVMFS_cms_cern_ch && BOSCOCluster == "ce03.cmsaf.mit.edu"
job_lease_duration = 240
should_transfer_files = YES
when_to_transfer_output = ON_EXIT
transfer_input_files = skim.exe,mva.tgz,/tmp/$PROXYFILE
Queue 
EOF

condor_submit skim-${tag}.condor -name submit.mit.edu
# condor_submit -pool submit.mit.edu:9615 -name submit.mit.edu -spool skim-${tag}.condor
mv skim-${tag}.condor $LOGDIR/log-${infn}.condor
counter=$(($counter+1))
        fi
    fi
done

echo -e "Submitted \033[1;36m$counter\033[0m jobs to Condor."
