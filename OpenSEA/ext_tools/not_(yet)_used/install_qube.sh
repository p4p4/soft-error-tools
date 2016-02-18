#!/bin/bash

if [ "$DEMIURGETP" = "" ]
then
   echo "The Environment variable DEMIURGETP is undefined."
   exit 1
fi

echo "Installing the QuBE QBF solver ..."

QUBE_ACHRIVE="QuBE7.2.gz"

if [ ! -e "$QUBE_ACHRIVE" ];
then
  echo " Downloading the QuBE QBF solver ..."
  wget http://www.star.dist.unige.it/%7Eqube/Download/Data/Solver/$QUBE_ACHRIVE
fi

echo " Unpacking the QuBE QBF solver ..."
mkdir -p $DEMIURGETP/qube
rm -f $DEMIURGETP/qube/qube
gunzip -c $QUBE_ACHRIVE > $DEMIURGETP/qube/qube
chmod +x $DEMIURGETP/qube/qube


