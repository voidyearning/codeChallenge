#!/usr/bin/env bash

# example of the run script for running the rolling_median calculation with a python file, 
# but could be replaced with similar files from any major language

if [ ! -e medianDegree.out ] 
then
    g++ src/main.cpp src/VenmoGraph.cpp -o medianDegree.out
fi
./medianDegree.out


