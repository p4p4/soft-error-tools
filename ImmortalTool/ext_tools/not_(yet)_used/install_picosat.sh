#!/bin/bash

if [ "$DEMIURGETP" = "" ]
then
   echo "The Environment variable DEMIURGETP is undefined."
   exit 1
fi

echo "Installing PicoSAT ..."

PICOSAT="picosat-960"
PICOSAT_ACHRIVE="$PICOSAT.tar.gz"

if [ ! -e "$PICOSAT_ACHRIVE" ];
then
  echo " Downloading PicoSAT ..."
  wget http://fmv.jku.at/picosat/$PICOSAT_ACHRIVE
fi

echo " Unpacking PicoSAT ..."
rm -rf $DEMIURGETP/picosat
tar -xzf $PICOSAT_ACHRIVE -C $DEMIURGETP
mv $DEMIURGETP/$PICOSAT $DEMIURGETP/picosat

echo " Compiling PicoSAT ..."
cd $DEMIURGETP/picosat
./configure -t
make


