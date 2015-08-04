#!/bin/bash

if [ "$DEMIURGETP" = "" ]
then
   echo "The Environment variable DEMIURGETP is undefined."
   exit 1
fi

echo "Installing MiniSat ..."

MINISAT="minisat-2.2.0"
MINISAT_ACHRIVE="$MINISAT.tar.gz"

if [ ! -e "$MINISAT_ACHRIVE" ];
then
  echo " Downloading MiniSat ..."
  wget http://minisat.se/downloads/$MINISAT_ACHRIVE
fi

echo " Unpacking MiniSat ..."
rm -rf $DEMIURGETP/minisat
tar -xzf $MINISAT_ACHRIVE -C $DEMIURGETP

echo " Compiling MiniSat ..."
export MROOT=$DEMIURGETP/minisat
cd $DEMIURGETP/minisat/core
make rs
make libr
