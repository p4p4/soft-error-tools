#!/bin/bash

if [ "$IMMORTALTP" = "" ]
then
   echo "The Environment variable IMMORTALTP is undefined."
   exit 1
fi

echo "Installing the CUDD BDD library ..."

CUDD="cudd-3.0.0"
CUDD_ARCHIVE="$CUDD.tar.gz"



if [ ! -e "$CUDD_ARCHIVE" ];
then
  echo " Downloading the CUDD BDD library ..."
  wget ftp://vlsi.colorado.edu/pub/$CUDD_ARCHIVE
fi

echo " Unpacking the CUDD BDD library ..."
rm -rf $IMMORTALTP/cudd
tar -xzf $CUDD_ARCHIVE -C $IMMORTALTP
mv $IMMORTALTP/$CUDD $IMMORTALTP/cudd

echo " Compiling the CUDD BDD library ..."
cd $IMMORTALTP/cudd
./configure --enable-obj
make
