#!/bin/bash

if [ "$DEMIURGETP" = "" ]
then
   echo "The Environment variable DEMIURGETP is undefined."
   exit 1
fi

echo "Copying the syntactic checker ..."
cp syntactic_checker.py $DEMIURGETP/syntactic_checker.py

