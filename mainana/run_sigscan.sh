#!/bin/bash

g++ djcomp_significance.cc $(root-config --libs --cflags) -I"../includes/" -g -o djcomp_significance.exe || exit 1

./djcomp_significance.exe

rm djcomp_significance.exe
