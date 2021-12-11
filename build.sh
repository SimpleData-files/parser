#!/bin/bash

rm simpledata &> /dev/null
if [ $# == 0 ]
then
    read -p "Enter a C++ compiler to use: " compiler
else
    compiler=$1
fi

$compiler parser.cpp -o simpledata

if [ -f simpledata ]
then
    echo "Compilation Successful!"
fi