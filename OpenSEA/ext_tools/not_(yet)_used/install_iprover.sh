#!/bin/bash

if [ "$DEMIURGETP" = "" ]
then
   echo "The Environment variable DEMIURGETP is undefined."
   exit 1
fi

echo "Installing iProver ..."

cd $DEMIURGETP
rm -rf iprover
mkdir iprover
cd iprover
echo " Downloading iProver from GIT repository ..."
git clone https://code.google.com/p/iprover/
cd iprover
echo " Compiling iProver ..."
./configure
make




