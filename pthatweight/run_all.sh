#!/bin/bash

ichan=(0)
###
filelists=(
    "/raid5/data/wangj/DntupleRun2018/phodmva_20200926_Dpho_20200924_QCDPhoton_pThat_*_Filter30GeV_TuneCP5_HydjetDrumMB_trk1Dpt2.root" # 0: QCDphoton + 30GeV filter
)
##
crosssec=(
    'const int nBins=4; float pthatBin[nBins]={30, 50, 80, 120}; float crosssec[nBins+1]={1002508., 341974., 88321., 21579., 0.};'
)

##
for ii in ${ichan[@]}
do
    ##
    tmp=$(date +%y%m%d%H%M%S)
    sed '1i'"${crosssec[$ii]}" weighPurePthat.C > weighPurePthat_${tmp}.C

    g++ addbranch.C $(root-config --cflags --libs) -I"../includes/" -g -o addbranch_${tmp}.exe || { rm weighPurePthat_${tmp}.C ; exit 1 ; }
    g++ weighPurePthat_${tmp}.C $(root-config --cflags --libs) -I"../includes/" -g -o weighPurePthat_${tmp}.exe || { rm weighPurePthat_${tmp}.C ; rm addbranch_${tmp}.exe ; exit 1 ; }

    filelist=${filelists[ii]}
    echo "=========== add sample pthat cut value >>>>"
    mergelist=
    for ifile in `echo $filelist`
    do
        ifilecp=${ifile%%.root}
        ifilecp=${ifilecp}_addSamplePthat.root
        pthatcut=${ifile##*pThat_} ; pthatcut=${pthatcut%%_*.root}
        echo "----------"
        echo "input:  $ifile"
        echo "output: $ifilecp"
        echo "pthatcut value: $pthatcut"
        [[ $ifile == $ifilecp ]] && { echo "invalid input for ./addbranch.exe" ; continue ; }
        [[ ${1:-0} -eq 1 ]] && {
            rsync --progress $ifile $ifilecp
            set -x
            yes y | ./addbranch_${tmp}.exe $ifilecp hiEvtAnalyzer/HiTree sample $pthatcut float
            set +x
        }
        mergelist="$mergelist "$ifilecp
    done

    echo

    ##
    echo "=========== merge >>>>"
    # mergesuffix=${filelist##*'Pthat-*'} ; mergesuffix=${mergesuffix%%.root}
    # mergepreffix=${filelist%%_Pthat*}
    mergesuffix=${filelist##*'pThat_*'} ; mergesuffix=${mergesuffix%%.root}
    mergepreffix=${filelist%%_pThat*}
    mergeoutput=$mergepreffix${mergesuffix}_addSamplePthat_noweight.root
    echo "----------"
    echo "input:  $mergelist"
    echo "output: $mergeoutput"
    [[ ${2:-0} -eq 1 ]] && {
        set -x
        hadd $mergeoutput $mergelist
        set +x
    }

    echo

    ##
    echo "=========== weight >>>>"
    weightoutput=$mergepreffix${mergesuffix}_addSamplePthat_pthatweight.root
    echo "----------"
    echo "input:  $mergeoutput"
    echo "output: $weightoutput"
    [[ ${3:-0} -eq 1 ]] && { 
        rsync --progress "$mergeoutput" "$weightoutput"
        set -x
        ./weighPurePthat_${tmp}.exe "$mergeoutput" "$weightoutput" 
        set +x
    }

    rm weighPurePthat_${tmp}.exe
    rm weighPurePthat_${tmp}.C
    rm addbranch_${tmp}.exe
done

