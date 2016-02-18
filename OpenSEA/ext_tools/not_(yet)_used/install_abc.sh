#!/bin/bash

if [ "$DEMIURGETP" = "" ]
then
   echo "The Environment variable DEMIURGETP is undefined."
   exit 1
fi

echo "Installing ABC ..."

cd $DEMIURGETP
rm -rf abc
mkdir abc
cd abc
echo " Downloading ABC from GIT repository ..."
# -r d3db71b fixes the version to the one commited on 2014-05-19
hg clone -r d3db71b https://bitbucket.org/alanmi/abc
cd abc
echo " Compiling ABC ..."
make

