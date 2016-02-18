#!/bin/bash

if [ "$DEMIURGETP" = "" ]
then
   echo "The Environment variable DEMIURGETP is undefined."
   exit 1
fi

echo "Installing DepQBF ..."

DEPQBF_ACHRIVE="version-3.04.tar.gz"

if [ ! -e "$DEPQBF_ACHRIVE" ];
then
  echo " Downloading DepQBF ..."
  wget https://github.com/lonsing/depqbf/archive/$DEPQBF_ACHRIVE
fi

echo " Unpacking DepQBF ..."
rm -rf $DEMIURGETP/depqbf
tar -xzf $DEPQBF_ACHRIVE -C $DEMIURGETP

mv $DEMIURGETP/depqbf-version-3.04 $DEMIURGETP/depqbf
 
echo " Compiling DepQBF ..."
(cd $DEMIURGETP/depqbf; make)
