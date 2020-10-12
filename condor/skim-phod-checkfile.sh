#!/bin/bash

if [[ $# -ne 8 ]]; then
    echo "usage: ./skim-phod-checkfile.sh [input file] [output dir] [output filename] [ishi] [evtfilt] [hltfilt] [mvafilt] [proxy]"
    exit 1
fi

INFILE=$1
DESTINATION=$2
OUTFILE=$3
ISHI=$4
EVTFILT=$5
MVAFILT=$6
HLTFILT=$7
export X509_USER_PROXY=${PWD}/$8

SRM_PREFIX="/mnt/hadoop/"
SRM_PATH=${DESTINATION#${SRM_PREFIX}}

#FILE=$(head -n$(($1+1)) $2 | tail -n1)
tar -xzvf mva.tgz

echo ./skim.exe $INFILE $OUTFILE
./skim.exe $INFILE $OUTFILE $ISHI $EVTFILT $MVAFILT $HLTFILT

if [[ $? -eq 0 ]]; then
    # gfal-copy file://${PWD}/${OUTFILE}  srm://se01.cmsaf.mit.edu:8443/srm/v2/server?SFN=${DESTINATION}/${OUTFILE}
    set -x
    LD_LIBRARY_PATH='' PYTHONPATH='' gfal-copy file://$PWD/${OUTFILE} gsiftp://se01.cmsaf.mit.edu:2811/${SRM_PATH}/${OUTFILE}
    # mv $OUTFILE $DESTINATION/
    set +x
fi

rm $OUTFILE
rm mva.tgz
rm -r mva