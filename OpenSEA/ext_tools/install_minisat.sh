#!/bin/bash

if [ "$IMMORTALTP" = "" ]
then
   echo "The Environment variable IMMORTALTP is undefined."
   exit 1
fi

echo "Installing MiniSat ..."

if [ ! -e "$MINISAT_ACHRIVE" ];
then
  echo " Downloading MiniSat ..."
  rm -rf $IMMORTALTP/minisat
  git clone https://github.com/stp/minisat.git $IMMORTALTP/minisat
fi

echo " Compiling MiniSat ..."
cd $IMMORTALTP/minisat
make config prefix=$IMMORTALTP/minisat
make all
