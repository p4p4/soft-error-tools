#!/bin/bash

if [ "$IMMORTALTP" = "" ]
then
   echo "The Environment variable IMMORTALTP is undefined."
   exit 1
fi

echo "Installing the CUDD BDD library ..."

CUDD="cudd-2.5.1"
CUDD_ACHRIVE="$CUDD.tar.gz"



if [ ! -e "$CUDD_ACHRIVE" ];
then
  echo " Downloading the CUDD BDD library ..."
  wget ftp://vlsi.colorado.edu/pub/$CUDD_ACHRIVE
fi

echo " Unpacking the CUDD BDD library ..."
rm -rf $IMMORTALTP/cudd
tar -xzf $CUDD_ACHRIVE -C $IMMORTALTP
mv $IMMORTALTP/$CUDD $IMMORTALTP/cudd

echo " Compiling the CUDD BDD library ..."
cd $IMMORTALTP/cudd
make build
make cudd
make obj
make objlib

