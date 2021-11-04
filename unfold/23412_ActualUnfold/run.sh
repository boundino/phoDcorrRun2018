#!/bin/bash

./compile.sh
g++ draw.cpp $(root-config --libs --cflags) -I"../../includes/" -g -o draw.exe || exit 2 ;

for input in `echo $@`
do
    echo $input
    [[ x$input == x ]] && exit 1 ;

    tag=${input##"Input/unfold_"}
    tag=${tag%%.root}
    echo $tag

    # ./Execute --MCPrior true --Input $input --Output "Output/Unfolded_"$tag".root"
    ./Execute --Input $input --Output "Output/Unfolded_"$tag".root"
    # ./ExecutePlot --Input "Output/Unfolded_"$tag".root" --Original $input --Output "plots/Unfolded_"${tag}".pdf"
    ./draw.exe "Output/Unfolded_"$tag".root" $input "Unfolded_"$tag
done
