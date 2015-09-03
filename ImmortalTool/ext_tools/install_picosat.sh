#!/bin/bash

if [ "$IMMORTALTP" = "" ]
then
   echo "The Environment variable IMMORTALTP is undefined."
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
rm -rf $IMMORTALTP/picosat
tar -xzf $PICOSAT_ACHRIVE -C $IMMORTALTP
mv $IMMORTALTP/$PICOSAT $IMMORTALTP/picosat

echo " Compiling PicoSAT ..."
cd $IMMORTALTP/picosat
./configure -t
make


