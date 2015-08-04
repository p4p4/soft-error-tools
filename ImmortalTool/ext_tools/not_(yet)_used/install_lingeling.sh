#!/bin/bash

if [ "$DEMIURGETP" = "" ]
then
   echo "The Environment variable DEMIURGETP is undefined."
   exit 1
fi

echo "Installing the Lingeling SAT-solver ..."

LINGELING="lingeling-ayv-86bf266-140429"
LINGELING_ACHRIVE="$LINGELING.zip"

if [ ! -e "$LINGELING_ACHRIVE" ];
then
  echo " Downloading Lingeling ..."
  wget http://fmv.jku.at/lingeling/$LINGELING_ACHRIVE
fi

echo " Unpacking Lingeling ..."
rm -rf $DEMIURGETP/lingeling
unzip $LINGELING_ACHRIVE -d $DEMIURGETP/lingeling

echo " Compiling Lingeling ..."
cd $DEMIURGETP/lingeling/code
./configure.sh --aiger=$DEMIURGETP/aiger-1.9.4/
make

