#!/bin/sh

if [ "$IMMORTALTP" = "" ]
then
   echo "The Environment variable IMMORTALTP is undefined."
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

(cd $CPPUNIT; ./configure --prefix=$IMMORTALTP/$CPPUNIT; make; make check; make install)


