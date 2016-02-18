#!/bin/bash

if [ "$IMMORTALTP" = "" ]
then
   echo "The Environment variable IMMORTALTP is undefined."
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
rm -rf $IMMORTALTP/$AIGER
tar -xzf $AIGER_ACHRIVE -C $IMMORTALTP

echo " Compiling AIGER utilities ..."
(cd $IMMORTALTP/$AIGER; ./configure && make)
