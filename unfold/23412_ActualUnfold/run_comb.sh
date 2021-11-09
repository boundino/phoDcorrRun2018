#!/bin/bash

RUN=${1:-0}

g++ combine.cpp $(root-config --libs --cflags) -I"../../includes/" -g -o combine.exe || exit 1

[[ $RUN -eq 1 ]] && ./combine.exe "dphi"
[[ $RUN -eq 1 ]] && ./combine.exe "dr"
