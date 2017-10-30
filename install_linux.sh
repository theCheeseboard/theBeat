#!/bin/bash

theBeat_dir=`pwd`

cd ~/tmp
git clone https://github.com/vicr123/the-libs
cd the-libs
qmake
make
pkexec make install

cd $theBeat_dir
qmake
make
pkexec make install

exit 


