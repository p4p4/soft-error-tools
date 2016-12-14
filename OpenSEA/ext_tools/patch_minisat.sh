#!/bin/bash
#
# TODO: this is just a quick-fix to be able to compile OpenSEA again with newer versions of gcc
#
# !!! IMPORTANT !!!:
#
# only run this if "make" in OpenSEA fails with the following error message:
#   minisat/minisat/mtl/Map.h:32:99: error: missing template arguments before ‘(’ token
#   late<class K> struct Hash  { uint32_t operator()(const K& k)               const { return hash(k);  } };
#                                                                                              ^
#
   
echo " Patching MiniSat ..."
sed -i -e 's/               const { return hash/               const { return hash<K>/g' "$IMMORTALTP/minisat/minisat/mtl/Map.h"
