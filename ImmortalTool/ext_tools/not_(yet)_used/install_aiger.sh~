#!/bin/bash

if [ "$DEMIURGETP" = "" ]
then
   echo "The Environment variable DEMIURGETP is undefined."
   exit 1
fi

echo "Installing AIGER utilities ..."

AIGER="aiger-1.9.4"
AIGER_ACHRIVE="$AIGER.tar.gz"

if [ ! -e "$AIGER_ACHRIVE" ];
then
  echo " Downloading AIGER utilities ..."
  wget http://fmv.jku.at/aiger/$AIGER_ACHRIVE
fi

echo " Unpacking AIGER utilities ..."
rm -rf $DEMIURGETP/$AIGER
tar -xzf $AIGER_ACHRIVE -C $DEMIURGETP

echo " Compiling AIGER utilities ..."
(cd $DEMIURGETP/$AIGER; ./configure && make)
