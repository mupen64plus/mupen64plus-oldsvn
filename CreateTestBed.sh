#!/bin/sh
rm ./main/main.o
printf "\e[0;31mUpdating Executable...\e[m\n"
make PLAT=win32 BITS=32 NO_RESAMP=1 DBGSYM=1 all
printf "\e[0;31mSetting up testbed...\e[m\n"
cp plugins/ config/ ./TestBed/ -r; cp mupen64plus.exe ./TestBed/
printf "\e[0;31mClearing junk...\e[m\n"
rm -rf `find ./TestBed/ -type d -name .svn`
printf "\e[0;34mDone!\e[m\n"
