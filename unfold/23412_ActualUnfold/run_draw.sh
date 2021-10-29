#!/bin/bash

g++ drawInput.cpp $(root-config --libs --cflags) -I"../..//includes/" -g -o drawInput.exe || exit 2 ;

for input in `echo $@`
do
    echo $input
    [[ x$input == x ]] && exit 1 ;

    tag=${input##"Input/unfold_"}
    tag=${tag%%.root}
    echo $tag

    ./drawInput.exe $input "drawInput_"$tag
done
