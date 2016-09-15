#!/bin/bash          
ABC=$IMMORTALTP/abc/abc/abc
A2A=$IMMORTALTP/aiger-1.9.4/aigtoaig

SRC=./adders/
TARGET=adders_aig/

IN_FILES=(
#0_two_modules
#1_two_modules_two_instances
#2_multiple_instances
5_pipeline_param
6_pipeline
7_pipeline
8_pipeline
#b
)

echo "=============================="
echo "Converting the benchmarks ..."

rm -rf $TARGET* # just to clean up
rm -rf $SRC*.mv # just to clean up
for filename in "${IN_FILES[@]}"
do
  mkdir -p `dirname $TARGET$filename`


  #$ABC -c "read_verilog $SRC$filename.v; strash; zero; write_aiger $TARGET$filename.aig"
  #yosys -o $TARGET$filename.blif -S $SRC$filename.v

  python jinja.py $SRC$filename.v $SRC$filename.unrolled.v
  vl2mv $SRC$filename.unrolled.v
  $ABC -c "read_blif_mv $SRC$filename.unrolled.mv; write_aiger -s $TARGET$filename.aig"
  #$ABC -c "read_blif $TARGET$filename.blif; strash; zero; write_aiger $TARGET$filename.aig"


  echo "The verilog file $filename.v has been converted using the following commands:" >> $TARGET$filename.aig

  echo "vl2mv $filename.v       # generates $filename.mv" >> $TARGET$filename.aig
  echo 'abc -c "read_blif_mv '$filename.mv'; write_aiger -s '$filename.aig'"' >> $TARGET$filename.aig

  echo "$filename.v:" >> $TARGET$filename.aig
  echo "___________________________" >> $TARGET$filename.aig
  cat $SRC$filename.v >> $TARGET$filename.aig

  $A2A $TARGET$filename.aig $TARGET$filename.aag
done

rm $TARGET*.aig # remove binary files

echo "=============================="
echo "Statistics:"
for filename in "${IN_FILES[@]}"
do
  if [ -e "$TARGET$filename.aig" ];
  then
    echo -n "$TARGET$filename.aig: "
    head -n 1 $TARGET$filename.aig
  fi
done
