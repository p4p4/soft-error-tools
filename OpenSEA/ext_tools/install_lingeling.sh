#!/bin/bash

if [ "$IMMORTALTP" = "" ]
then
   echo "The Environment variable IMMORTALTP is undefined."
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
rm -rf $IMMORTALTP/lingeling
unzip $LINGELING_ACHRIVE -d $IMMORTALTP/lingeling

echo " Compiling Lingeling ..."
cd $IMMORTALTP/lingeling/code
./configure.sh --aiger=$IMMORTALTP/aiger-1.9.4/
make

