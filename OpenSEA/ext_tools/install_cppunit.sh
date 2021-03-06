#!/bin/sh

if [ "$IMMORTALTP" = "" ]
then
   echo "The Environment variable IMMORTALTP is undefined."
   exit 1
fi

echo "Installing CPPUnit ..."

VERSION="1.13.2"
CPPUNIT="cppunit-$VERSION"
if [ ! -e "$CPPUNIT.tar.gz" ];
then
   wget http://dev-www.libreoffice.org/src/$CPPUNIT.tar.gz
fi
tar xzvf $CPPUNIT.tar.gz

export LDFLAGS="-ldl"

(cd $CPPUNIT; ./configure --prefix=$IMMORTALTP/$CPPUNIT; make; make check; make install)


