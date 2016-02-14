#!/bin/bash

if [ "$DEMIURGETP" = "" ]
then
   echo "The Environment variable DEMIURGETP is undefined."
   exit 1
fi

echo "Installing QBFcert ..."

QBFCERT_ACHRIVE="qbfcert-1.0.tar.gz"

if [ ! -e "$QBFCERT_ACHRIVE" ];
then
  echo " Downloading QBFcert ..."
  wget http://fmv.jku.at/qbfcert/$QBFCERT_ACHRIVE
fi

echo " Unpacking QBFcert ..."
rm -rf $DEMIURGETP/qbfcert
tar -xzf $QBFCERT_ACHRIVE -C $DEMIURGETP
cp qbfcert_min.sh $DEMIURGETP/qbfcert/qbfcert_min.sh
