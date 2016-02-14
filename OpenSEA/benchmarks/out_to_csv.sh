#!/bin/bash
for filename in *.out; do
  epsname=${filename%.out}.csv
  echo "processing $filename $epsname"
  python log_to_table.py $filename $epsname
done
