#!/bin/bash

input=${1}

g++ drawInput.cpp $(root-config --libs --cflags) -I"../../includes/" -g -o drawInput.exe || exit

output=${input%%.*}
output=${output##*/}

echo $output
./drawInput.exe $input $output
