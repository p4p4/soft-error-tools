#!/bin/bash
# dependencies:
# sudo apt-get install libreadline-dev mercurial
if [ "$IMMORTALTP" = "" ]
then
   echo "The Environment variable IMMORTALTP is undefined."
   exit 1
fi

echo "Installing ABC ..."

cd $IMMORTALTP
rm -rf abc
mkdir abc
cd abc
echo " Downloading ABC from GIT repository ..."
# -r d3db71b fixes the version to the one commited on 2014-05-19
hg clone https://bitbucket.org/alanmi/abc
cd abc
echo " Compiling ABC ..."
make

