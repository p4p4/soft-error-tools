#!/bin/bash

if [ "$DEMIURGETP" = "" ]
then
   echo "The Environment variable DEMIURGETP is undefined."
   exit 1
fi

echo "Copying QBFcert ..."
rm -rf $DEMIURGETP/qbfcert
cp -r qbfcert $DEMIURGETP

