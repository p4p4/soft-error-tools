#!/bin/bash

if [ "$DEMIURGETP" = "" ]
then
   echo "The Environment variable DEMIURGETP is undefined."
   exit 1
fi

echo "Installing the BLIMC model checker ..."

BLIMC="lingeling-ala-b02aa1a-121013"
BLIMC_ACHRIVE="$BLIMC.tar.gz"

if [ ! -e "$BLIMC_ACHRIVE" ];
then
  echo " Downloading the BLIMC model checker ..."
  wget http://fmv.jku.at/lingeling/$BLIMC_ACHRIVE
fi

echo " Unpacking the BLIMC model checker ..."
rm -rf $DEMIURGETP/blimc
tar -xzf $BLIMC_ACHRIVE -C $DEMIURGETP
mv $DEMIURGETP/$BLIMC $DEMIURGETP/blimc

echo " Compiling the BLIMC model checker ..."
cd $DEMIURGETP/blimc
./configure
cp $DEMIURGETP/blimc/makefile $DEMIURGETP/blimc/makefile_orig
sed 's/AIGER=/AIGER=$(DEMIURGETP)\/aiger-1.9.4/' $DEMIURGETP/blimc/makefile_orig > $DEMIURGETP/blimc/makefile
make
make blimc


