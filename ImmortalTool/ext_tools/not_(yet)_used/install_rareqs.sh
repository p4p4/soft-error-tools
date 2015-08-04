#!/bin/bash

if [ "$DEMIURGETP" = "" ]
then
   echo "The Environment variable DEMIURGETP is undefined."
   exit 1
fi

echo "Installing RAReQS ..."

RAREQS_ACHRIVE="rareqs-1.1.src.tgz"

if [ ! -e "$RAREQS_ACHRIVE" ];
then
  echo " Downloading RAReQS ..."
  wget http://sat.inesc-id.pt/~mikolas/sw/areqs/$RAREQS_ACHRIVE
fi

echo " Unpacking RAReQS ..."
rm -rf $DEMIURGETP/rareqs
tar -xzf $RAREQS_ACHRIVE -C $DEMIURGETP

mv $DEMIURGETP/rareqs-1.1 $DEMIURGETP/rareqs
cp ./rareqs_api.hh $DEMIURGETP/rareqs/rareqs_api.hh
cp ./rareqs_api.cc $DEMIURGETP/rareqs/rareqs_api.cc
 
echo " Compiling RAReQS ..."
cd $DEMIURGETP/rareqs
# to avoid a name clash:
sed -i 's/Options/OptionsR/g' *.cc
sed -i 's/Options/OptionsR/g' *.hh
mv Options.hh OptionsR.hh
mv Options.cc OptionsR.cc
make

echo " Making a library ..."
cd $DEMIURGETP/rareqs
ar rcs librareqs.a LitSet.o ObjectCounter.o qtypes.o RASolverLeaf.o RASolver.o ReadQ.o Univ.o VarVector.o OptionsR.o rareqs_api.o RASolverNoIt.o Reader.o Tests.o utils.o minisat_auxiliary.o Preprocess.o RASolverIt.o RASolverNonLeaf.o ReadException.o Unit.o VarSet.o ./minisat/core/libminisat.a
