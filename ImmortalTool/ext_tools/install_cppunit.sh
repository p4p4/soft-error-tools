#!/bin/sh

if [ "$DEMIURGETP" = "" ]
then
   echo "The Environment variable DEMIURGETP is undefined."
   exit 1
fi

echo "Installing CPPUnit ..."

VERSION="1.12.1"
CPPUNIT="cppunit-$VERSION"
if [ ! -e "$CPPUNIT.tar.gz" ];
then
   wget -O $CPPUNIT.tar.gz http://sourceforge.net/projects/cppunit/files/cppunit/$VERSION/$CPPUNIT.tar.gz/download
fi
tar xzvf $CPPUNIT.tar.gz

export LDFLAGS="-ldl"

(cd $CPPUNIT; ./configure --prefix=$DEMIURGETP/$CPPUNIT; make; make check; make install)


