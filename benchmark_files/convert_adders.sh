#!/bin/bash          
ABC=$IMMORTALTP/abc/abc/abc
A2A=$IMMORTALTP/aiger-1.9.4/aigtoaig

SRC=./own/
TARGET=own/AIG/

IN_FILES=(
pipe_add_i1_l1
pipe_add_i1_l2
pipe_add_i1_l4
pipe_add_i1_l6
pipe_add_i1_l8
pipe_add_i1_l10
pipe_add_i2_l1
pipe_add_i2_l2
pipe_add_i2_l4
pipe_add_i2_l6
pipe_add_i2_l8
pipe_add_i2_l10
pipe_add_i8_l1
pipe_add_i8_l2
pipe_add_i8_l4
pipe_add_i8_l6
pipe_add_i8_l8
pipe_add_i8_l10
pipe_add_i16_l1
pipe_add_i16_l2
pipe_add_i16_l4
pipe_add_i16_l6
pipe_add_i16_l8
pipe_add_i16_l10
pipe_add_i32_l1
pipe_add_i32_l2
pipe_add_i32_l4
pipe_add_i32_l6
pipe_add_i32_l8
pipe_add_i32_l10
pipe_add_i64_l1
pipe_add_i64_l2
pipe_add_i64_l4
pipe_add_i64_l6
pipe_add_i64_l8
pipe_add_i64_l10
pipe_add_i128_l1
pipe_add_i128_l2
pipe_add_i128_l4
pipe_add_i128_l6
pipe_add_i128_l8
pipe_add_i128_l10
pipe_add_tmr_i1_l1
pipe_add_tmr_i1_l2
pipe_add_tmr_i1_l4
pipe_add_tmr_i1_l6
pipe_add_tmr_i1_l8
pipe_add_tmr_i1_l10
pipe_add_tmr_i2_l1
pipe_add_tmr_i2_l2
pipe_add_tmr_i2_l4
pipe_add_tmr_i2_l6
pipe_add_tmr_i2_l8
pipe_add_tmr_i2_l10
pipe_add_tmr_i8_l1
pipe_add_tmr_i8_l2
pipe_add_tmr_i8_l4
pipe_add_tmr_i8_l6
pipe_add_tmr_i8_l8
pipe_add_tmr_i8_l10
pipe_add_tmr_i16_l1
pipe_add_tmr_i16_l2
pipe_add_tmr_i16_l4
pipe_add_tmr_i16_l6
pipe_add_tmr_i16_l8
pipe_add_tmr_i16_l10
pipe_add_tmr_i32_l1
pipe_add_tmr_i32_l2
pipe_add_tmr_i32_l4
pipe_add_tmr_i32_l6
pipe_add_tmr_i32_l8
pipe_add_tmr_i32_l10
pipe_add_tmr_i64_l1
pipe_add_tmr_i64_l2
pipe_add_tmr_i64_l4
pipe_add_tmr_i64_l6
pipe_add_tmr_i64_l8
pipe_add_tmr_i64_l10
pipe_add_tmr_i128_l1
pipe_add_tmr_i128_l2
pipe_add_tmr_i128_l4
pipe_add_tmr_i128_l6
pipe_add_tmr_i128_l8
pipe_add_tmr_i128_l10
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

#rm $TARGET*.aig # remove binary files

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
