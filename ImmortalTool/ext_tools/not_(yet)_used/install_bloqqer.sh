#!/bin/bash

if [ "$DEMIURGETP" = "" ]
then
   echo "The Environment variable DEMIURGETP is undefined."
   exit 1
fi

echo "Installing Bloqqer ..."

echo " Copying Bloqqer ..."
rm -rf $DEMIURGETP/bloqqer
cp -R ./bloqqer $DEMIURGETP/bloqqer
cd $DEMIURGETP/bloqqer
./configure -c
make
make libbloqqer.a
