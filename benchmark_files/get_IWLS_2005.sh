#!/bin/bash          
ABC=/home/pklampfl/immortal/abc/abc/abc
SRC=./IWLS_benchmarks_2005_V_1.0/
TARGET=IWLS_2005_AIG/

IN_FILES=(
iwls2005_blif/ac97_ctrl
iwls2005_blif/aes_core
iwls2005_blif/des_area
iwls2005_blif/des_perf
iwls2005_blif/ethernet
iwls2005_blif/i2c
iwls2005_blif/mem_ctrl
iwls2005_blif/pci_bridge32
iwls2005_blif/pci_conf_cyc_addr_dec
iwls2005_blif/pci_spoci_ctrl
iwls2005_blif/sasc
iwls2005_blif/simple_spi
iwls2005_blif/spi
iwls2005_blif/ss_pcm
iwls2005_blif/steppermotordrive
iwls2005_blif/systemcaes
iwls2005_blif/systemcdes
iwls2005_blif/tv80
iwls2005_blif/usb_funct
iwls2005_blif/usb_phy
iwls2005_blif/vga_lcd
iwls2005_blif/wb_conmax
iwls2005_blif/wb_dma
iwls2005_large_blif/leon2
iwls2005_large_blif/leon3
iwls2005_large_blif/leon3mp
iwls2005_large_blif/netcard
IWLS_benchmarks_2005_V_1.0/iscas/blif/s27
IWLS_benchmarks_2005_V_1.0/iscas/blif/s208
IWLS_benchmarks_2005_V_1.0/iscas/blif/s298
IWLS_benchmarks_2005_V_1.0/iscas/blif/s344
IWLS_benchmarks_2005_V_1.0/iscas/blif/s349
IWLS_benchmarks_2005_V_1.0/iscas/blif/s382
IWLS_benchmarks_2005_V_1.0/iscas/blif/s386
IWLS_benchmarks_2005_V_1.0/iscas/blif/s400
IWLS_benchmarks_2005_V_1.0/iscas/blif/s420
IWLS_benchmarks_2005_V_1.0/iscas/blif/s444
IWLS_benchmarks_2005_V_1.0/iscas/blif/s510
IWLS_benchmarks_2005_V_1.0/iscas/blif/s526
IWLS_benchmarks_2005_V_1.0/iscas/blif/s526n
IWLS_benchmarks_2005_V_1.0/iscas/blif/s641
IWLS_benchmarks_2005_V_1.0/iscas/blif/s713
IWLS_benchmarks_2005_V_1.0/iscas/blif/s820
IWLS_benchmarks_2005_V_1.0/iscas/blif/s832
IWLS_benchmarks_2005_V_1.0/iscas/blif/s838
IWLS_benchmarks_2005_V_1.0/iscas/blif/s953
IWLS_benchmarks_2005_V_1.0/iscas/blif/s1196
IWLS_benchmarks_2005_V_1.0/iscas/blif/s1238
IWLS_benchmarks_2005_V_1.0/iscas/blif/s1423
IWLS_benchmarks_2005_V_1.0/iscas/blif/s1488
IWLS_benchmarks_2005_V_1.0/iscas/blif/s1494
IWLS_benchmarks_2005_V_1.0/iscas/blif/s5378
IWLS_benchmarks_2005_V_1.0/iscas/blif/s9234
IWLS_benchmarks_2005_V_1.0/iscas/blif/s13207
IWLS_benchmarks_2005_V_1.0/iscas/blif/s15850
IWLS_benchmarks_2005_V_1.0/iscas/blif/s35932
IWLS_benchmarks_2005_V_1.0/iscas/blif/s38417
IWLS_benchmarks_2005_V_1.0/iscas/blif/s38584
)

echo "=============================="
echo "Downloading the benchmarks ..."

if [ ! -e "iwls2005_blif.zip" ];
then
  wget http://www.eecs.berkeley.edu/~alanmi/benchmarks/iwls2005_blif.zip
fi
if [ ! -e "iwls2005_large_blif.zip" ];
then
  wget http://www.eecs.berkeley.edu/~alanmi/benchmarks/iwls2005_large_blif.zip
fi
if [ ! -e "IWLS_2005_benchmarks_V_1.0.tgz" ];
then
  wget http://iwls.org/iwls2005/IWLS_2005_benchmarks_V_1.0.tgz
fi

if [ ! -d "iwls2005_blif" ];
then
  unzip iwls2005_blif.zip -d iwls2005_blif
fi 
if [ ! -d "iwls2005_large_blif" ];
then
  unzip iwls2005_large_blif.zip -d iwls2005_large_blif
fi
if [ ! -d "IWLS_benchmarks_2005_V_1.0" ];
then
  tar -xzf IWLS_2005_benchmarks_V_1.0.tgz
fi

echo "=============================="
echo "Converting the benchmarks ..."
rm -rf $TARGET # just to clean up
mkdir -p $TARGET
for filename in "${IN_FILES[@]}"
do
  NAME_ONLY=`basename $filename`
  $ABC -c "read_blif $filename.blif; strash; zero; write_aiger $TARGET$NAME_ONLY.aig"
done

echo "=============================="
echo "Statistics:"
for filename in "${IN_FILES[@]}"
do
  NAME_ONLY=`basename $filename`
  if [ -e "$TARGET$NAME_ONLY.aig" ];
  then
    echo -n "$TARGET$NAME_ONLY.aig: "
    head -n 1 $TARGET$NAME_ONLY.aig
  fi
done
