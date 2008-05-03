#!/bin/sh
printf "\e[0;31mUpdating Executable...\e[m\n"
make PLAT=mingw BITS=32 NO_RESAMP=1 DBGSYM=1 all
printf "\e[0;31mSetting up testbed...\e[m\n"
cp plugins/ config/ mupen64plus.exe ./TestBed/ -r
printf "\e[0;31mClearing junk...\e[m\n"
rm -rf `find ./TestBed/ -type d -name .svn`
printf "\e[0;34mDone!\e[m\n"
