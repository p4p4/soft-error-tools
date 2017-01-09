#!/bin/sh

if [ "$IMMORTALTP" = "" ]
then
   echo "The Environment variable IMMORTALTP is undefined."
   exit 1
fi

echo "Installing GLU and VL2MV ..."

VERSION="2.4"
GLU="glu-$VERSION"
VL2MV="vl2mv-$VERSION"
GLU_ACHRIVE="$GLU.tar.gz"
VL2MV_ACHRIVE="$VL2MV.tar.gz"

if [ ! -e "$GLU_ACHRIVE" ];
then
   wget ftp://vlsi.colorado.edu/pub/vis/$GLU_ACHRIVE
fi

if [ ! -e "$VL2MV_ACHRIVE" ];
then
   wget ftp://vlsi.colorado.edu/pub/vis/$VL2MV_ACHRIVE
fi

rm -rf $IMMORTALTP/$GLU
tar -xzf $GLU_ACHRIVE -C $IMMORTALTP

rm -rf $IMMORTALTP/$VL2MV
tar -xzf $VL2MV_ACHRIVE -C $IMMORTALTP

cd $IMMORTALTP/$GLU
./configure
make

cd $IMMORTALTP/$VL2MV
./configure
make

