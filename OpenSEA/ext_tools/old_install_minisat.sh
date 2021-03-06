#!/bin/bash

if [ "$IMMORTALTP" = "" ]
then
   echo "The Environment variable IMMORTALTP is undefined."
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
rm -rf $IMMORTALTP/minisat
tar -xzf $MINISAT_ACHRIVE -C $IMMORTALTP

echo " Compiling MiniSat ..."
export MROOT=$IMMORTALTP/minisat
cd $IMMORTALTP/minisat/core
make rs
make libr

echo " Patching MiniSat ..." # TODO: this is just a quick-fix to be able to compile OpenSEA again.
sed -i -e 's/               const { return hash/               const { return hash<K>/g' "$IMMORTALTP/minisat/mtl/Map.h"
