#!/bin/bash

rm simpledata &> /dev/null
g++ parser.cpp -std=c++11 -o simpledata

if [ -f simpledata ]
then
    echo "Compilation Successful!"
fi
