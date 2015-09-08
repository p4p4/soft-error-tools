#!/bin/bash

if [ "$IMMORTALTP" = "" ]
then
   echo "The Environment variable IMMORTALTP is undefined."
   exit 1
fi

echo "Installing IC3 ..."

if [ ! -e "ic3.zip" ];
then
  echo " Downloading IC3 ..."
  # Tested with commit 8670762eaf downloaded on 2014-07-15
  wget https://github.com/arbrad/IC3ref/archive/master.zip
  mv master.zip ic3.zip
fi

if [ ! -e "minisat_ic3.zip" ];
then
  echo " Downloading Minisat for IC3 ..."
  # Tested with commit 37dc6c67e2, downloaded on 2014-07-15
  wget https://github.com/niklasso/minisat/archive/master.zip
  mv master.zip minisat_ic3.zip
fi

echo " Unpacking IC3 ..."
rm -rf $IMMORTALTP/ic3
unzip ic3.zip -d $IMMORTALTP/
mv $IMMORTALTP/IC3ref-master $IMMORTALTP/ic3
cp -r $IMMORTALTP/aiger-1.9.4/ $IMMORTALTP/ic3/aiger/
unzip minisat_ic3.zip -d $IMMORTALTP/ic3/
mv $IMMORTALTP/ic3/minisat-master $IMMORTALTP/ic3/minisat
echo " Patching IC3 ..."
sed -i 's/\/\/ create/aiger_reencode(aig); \/\/ create/g' $IMMORTALTP/ic3/main.cpp
sed -i 's/return 1;/return (rv?20:10);/g' $IMMORTALTP/ic3/main.cpp
echo " Compiling Minisat for IC3 ..."
# otherwise the compiler does not like this:
sed -i 's/PRIi64/ PRIi64/g' $IMMORTALTP/ic3/minisat/minisat/utils/Options.h
cd $IMMORTALTP/ic3/minisat/
make
echo " Compiling IC3 itself ..."
cd $IMMORTALTP/ic3/aiger/
make clean
cd $IMMORTALTP/ic3/
make





