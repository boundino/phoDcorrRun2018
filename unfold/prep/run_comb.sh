#!/bin/bash

vars=(
    `#0` "dphi"
    `#1` "dr"
)

run_combhist=${1:-0}

[[ $run_combhist -gt 0 || $# == 0 ]] && { g++ prep_combhist.cc $(root-config --libs --cflags) -I"../../includes/" -g -o prep_combhist.exe || exit 1 ; }

if [[ $run_combhist -eq 1 ]]
then
    for kvars in ${vars[@]}
    do
        ./prep_combhist.exe $kvars
    done
fi

rm prep_combhist.exe 2> /dev/null

