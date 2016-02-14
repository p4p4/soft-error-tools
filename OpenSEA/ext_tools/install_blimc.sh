#!/bin/bash

if [ "$IMMORTALTP" = "" ]
then
   echo "The Environment variable IMMORTALTP is undefined."
   exit 1
fi

echo "Installing the BLIMC model checker ..."

BLIMC="lingeling-ala-b02aa1a-121013"
BLIMC_ACHRIVE="$BLIMC.tar.gz"

if [ ! -e "$BLIMC_ACHRIVE" ];
then
  echo " Downloading the BLIMC model checker ..."
  wget http://fmv.jku.at/lingeling/$BLIMC_ACHRIVE
fi

echo " Unpacking the BLIMC model checker ..."
rm -rf $IMMORTALTP/blimc
tar -xzf $BLIMC_ACHRIVE -C $IMMORTALTP
mv $IMMORTALTP/$BLIMC $IMMORTALTP/blimc

echo " Compiling the BLIMC model checker ..."
cd $IMMORTALTP/blimc
./configure
cp $IMMORTALTP/blimc/makefile $IMMORTALTP/blimc/makefile_orig
sed 's/AIGER=/AIGER=$(IMMORTALTP)\/aiger-1.9.4/' $IMMORTALTP/blimc/makefile_orig > $IMMORTALTP/blimc/makefile
make
make blimc


