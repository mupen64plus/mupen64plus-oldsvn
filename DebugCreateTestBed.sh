#!/bin/sh
rm ./main/main.o
sh CreateTestBed.sh
cd TestBed
sh Debug.sh
cd ..