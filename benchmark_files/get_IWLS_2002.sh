#!/bin/bash          
ABC=$IMMORTALTP/abc/abc/abc
SRC=./iwls_1.0/benchmarks/
TARGET=IWLS_2002_AIG/

IN_FILES=(
iscas89/s27_orig
iscas89/s27_sweep
iscas89/s27_synth
iscas89/s208.1_orig
iscas89/s208.1_sweep
iscas89/s208.1_synth
iscas89/s298_orig
iscas89/s298_sweep
iscas89/s298_synth
iscas89/s344_orig
iscas89/s344_sweep
iscas89/s344_synth
iscas89/s349_orig
iscas89/s349_sweep
iscas89/s349_synth
iscas89/s382_orig
iscas89/s382_sweep
iscas89/s382_synth
iscas89/s386_orig
iscas89/s386_sweep
iscas89/s386_synth
iscas89/s400_orig
iscas89/s400_sweep
iscas89/s400_synth
iscas89/s420.1_orig
iscas89/s420.1_sweep
iscas89/s420.1_synth
iscas89/s444_orig
iscas89/s444_sweep
iscas89/s444_synth
iscas89/s510_orig
iscas89/s510_sweep
iscas89/s510_synth
iscas89/s526n_orig
iscas89/s526n_sweep
iscas89/s526n_synth
iscas89/s526_orig
iscas89/s526_sweep
iscas89/s526_synth
iscas89/s641_orig
iscas89/s641_sweep
iscas89/s641_synth
iscas89/s713_orig
iscas89/s713_sweep
iscas89/s713_synth
iscas89/s820_orig
iscas89/s820_sweep
iscas89/s820_synth
iscas89/s832_orig
iscas89/s832_sweep
iscas89/s832_synth
iscas89/s838.1_orig
iscas89/s838.1_sweep
iscas89/s838.1_synth
iscas89/s953_orig
iscas89/s953_sweep
iscas89/s953_synth
iscas89/s1196_orig
iscas89/s1196_sweep
iscas89/s1196_synth
iscas89/s1423_orig
iscas89/s1423_sweep
iscas89/s1423_synth
iscas89/s1488_orig
iscas89/s1488_sweep
iscas89/s1488_synth
iscas89/s1494_orig
iscas89/s1494_sweep
iscas89/s1494_synth
iscas89/s5378_orig
iscas89/s5378_sweep
iscas89/s9234.1_orig
iscas89/s9234.1_sweep
iscas89/s9234.1_synth
iscas89/s13207.1_orig
iscas89/s13207.1_sweep
iscas89/s13207.1_synth
iscas89/s15850.1_orig
iscas89/s15850.1_sweep
iscas89/s35932_orig
iscas89/s35932_sweep
iscas89/s38417_orig
iscas89/s38417_sweep
iscas89/s38417_synth
iscas89/s38584.1_orig
iscas89/s38584.1_sweep
iscas89/sbc_orig
iscas89/sbc_sweep
iscas89/sbc_synth
#
#
LGSynth89/9symml_orig
LGSynth89/9symml_sweep
LGSynth89/9symml_synth
LGSynth89/alu2_orig
LGSynth89/alu2_sweep
LGSynth89/alu2_synth
LGSynth89/alu4_orig
LGSynth89/alu4_sweep
LGSynth89/alu4_synth
LGSynth89/apex6_orig
LGSynth89/apex6_sweep
LGSynth89/apex6_synth
LGSynth89/apex7_orig
LGSynth89/apex7_sweep
LGSynth89/apex7_synth
LGSynth89/b1_orig
LGSynth89/b1_sweep
LGSynth89/b1_synth
LGSynth89/b9_orig
LGSynth89/b9_sweep
LGSynth89/b9_synth
LGSynth89/c8_orig
LGSynth89/c8_sweep
LGSynth89/c8_synth
LGSynth89/C17_orig
LGSynth89/C17_sweep
LGSynth89/C17_synth
LGSynth89/C432_orig
LGSynth89/C432_sweep
LGSynth89/C432_synth
LGSynth89/C499_orig
LGSynth89/C499_sweep
LGSynth89/C499_synth
LGSynth89/C880_orig
LGSynth89/C880_sweep
LGSynth89/C880_synth
LGSynth89/C1355_orig
LGSynth89/C1355_sweep
LGSynth89/C1355_synth
LGSynth89/C1908_orig
LGSynth89/C1908_sweep
LGSynth89/C1908_synth
LGSynth89/C2670_orig
LGSynth89/C2670_sweep
LGSynth89/C2670_synth
LGSynth89/C3540_orig
LGSynth89/C3540_sweep
LGSynth89/C3540_synth
LGSynth89/C5315_orig
LGSynth89/C5315_sweep
LGSynth89/C5315_synth
LGSynth89/C6288_orig
LGSynth89/C6288_sweep
LGSynth89/C6288_synth
LGSynth89/C7552_orig
LGSynth89/C7552_sweep
LGSynth89/C7552_synth
LGSynth89/cc_orig
LGSynth89/cc_sweep
LGSynth89/cc_synth
LGSynth89/cht_orig
LGSynth89/cht_sweep
LGSynth89/cht_synth
LGSynth89/cm42a_orig
LGSynth89/cm42a_sweep
LGSynth89/cm42a_synth
LGSynth89/cm82a_orig
LGSynth89/cm82a_sweep
LGSynth89/cm82a_synth
LGSynth89/cm85a_orig
LGSynth89/cm85a_sweep
LGSynth89/cm85a_synth
LGSynth89/cm138a_orig
LGSynth89/cm138a_sweep
LGSynth89/cm138a_synth
LGSynth89/cm150a_orig
LGSynth89/cm150a_sweep
LGSynth89/cm150a_synth
LGSynth89/cm151a_orig
LGSynth89/cm151a_sweep
LGSynth89/cm151a_synth
LGSynth89/cm152a_orig
LGSynth89/cm152a_sweep
LGSynth89/cm152a_synth
LGSynth89/cm162a_orig
LGSynth89/cm162a_sweep
LGSynth89/cm162a_synth
LGSynth89/cm163a_orig
LGSynth89/cm163a_sweep
LGSynth89/cm163a_synth
LGSynth89/cmb_orig
LGSynth89/cmb_sweep
LGSynth89/cmb_synth
LGSynth89/comp_orig
LGSynth89/comp_sweep
LGSynth89/comp_synth
LGSynth89/count_orig
LGSynth89/count_sweep
LGSynth89/count_synth
LGSynth89/cu_orig
LGSynth89/cu_sweep
LGSynth89/cu_synth
LGSynth89/decod_orig
LGSynth89/decod_sweep
LGSynth89/decod_synth
LGSynth89/des_orig
LGSynth89/des_sweep
LGSynth89/des_synth
LGSynth89/example2_orig
LGSynth89/example2_sweep
LGSynth89/example2_synth
LGSynth89/f51m_orig
LGSynth89/f51m_sweep
LGSynth89/f51m_synth
LGSynth89/frg1_orig
LGSynth89/frg1_sweep
LGSynth89/frg1_synth
LGSynth89/frg2_orig
LGSynth89/frg2_sweep
LGSynth89/frg2_synth
LGSynth89/k2_orig
LGSynth89/k2_sweep
LGSynth89/k2_synth
LGSynth89/lal_orig
LGSynth89/lal_sweep
LGSynth89/lal_synth
LGSynth89/ldd_orig
LGSynth89/ldd_sweep
LGSynth89/ldd_synth
LGSynth89/majority_orig
LGSynth89/majority_sweep
LGSynth89/majority_synth
LGSynth89/mux_orig
LGSynth89/mux_sweep
LGSynth89/mux_synth
LGSynth89/my_adder_orig
LGSynth89/my_adder_sweep
LGSynth89/my_adder_synth
LGSynth89/pair_orig
LGSynth89/pair_sweep
LGSynth89/pair_synth
LGSynth89/parity_orig
LGSynth89/parity_sweep
LGSynth89/parity_synth
LGSynth89/pcle_orig
LGSynth89/pcler8_orig
LGSynth89/pcler8_sweep
LGSynth89/pcler8_synth
LGSynth89/pcle_sweep
LGSynth89/pcle_synth
LGSynth89/pm1_orig
LGSynth89/pm1_sweep
LGSynth89/pm1_synth
LGSynth89/rot_orig
LGSynth89/rot_sweep
LGSynth89/rot_synth
LGSynth89/sct_orig
LGSynth89/sct_sweep
LGSynth89/sct_synth
LGSynth89/tcon_orig
LGSynth89/tcon_sweep
LGSynth89/tcon_synth
LGSynth89/term1_orig
LGSynth89/term1_sweep
LGSynth89/term1_synth
LGSynth89/too_large_orig
LGSynth89/too_large_sweep
LGSynth89/ttt2_orig
LGSynth89/ttt2_sweep
LGSynth89/ttt2_synth
LGSynth89/unreg_orig
LGSynth89/unreg_sweep
LGSynth89/unreg_synth
LGSynth89/vda_orig
LGSynth89/vda_sweep
LGSynth89/vda_synth
LGSynth89/x1_orig
LGSynth89/x1_sweep
LGSynth89/x1_synth
LGSynth89/x2_orig
LGSynth89/x2_sweep
LGSynth89/x2_synth
LGSynth89/x3_orig
LGSynth89/x3_sweep
LGSynth89/x3_synth
LGSynth89/x4_orig
LGSynth89/x4_sweep
LGSynth89/x4_synth
LGSynth89/z4ml_orig
LGSynth89/z4ml_sweep
LGSynth89/z4ml_synth
#
#
LGSynth91/cmlexamples/9symml_orig
LGSynth91/cmlexamples/9symml_sweep
LGSynth91/cmlexamples/9symml_synth
LGSynth91/cmlexamples/alu2_orig
LGSynth91/cmlexamples/alu2_sweep
LGSynth91/cmlexamples/alu2_synth
LGSynth91/cmlexamples/alu4_orig
LGSynth91/cmlexamples/alu4_sweep
LGSynth91/cmlexamples/alu4_synth
LGSynth91/cmlexamples/apex6_orig
LGSynth91/cmlexamples/apex6_sweep
LGSynth91/cmlexamples/apex6_synth
LGSynth91/cmlexamples/apex7_orig
LGSynth91/cmlexamples/apex7_sweep
LGSynth91/cmlexamples/apex7_synth
LGSynth91/cmlexamples/b1_orig
LGSynth91/cmlexamples/b1_sweep
LGSynth91/cmlexamples/b1_synth
LGSynth91/cmlexamples/b9_orig
LGSynth91/cmlexamples/b9_sweep
LGSynth91/cmlexamples/b9_synth
LGSynth91/cmlexamples/c8_orig
LGSynth91/cmlexamples/c8_sweep
LGSynth91/cmlexamples/c8_synth
LGSynth91/cmlexamples/C17_orig
LGSynth91/cmlexamples/C17_sweep
LGSynth91/cmlexamples/C17_synth
LGSynth91/cmlexamples/C432_orig
LGSynth91/cmlexamples/C432_sweep
LGSynth91/cmlexamples/C432_synth
LGSynth91/cmlexamples/C499_orig
LGSynth91/cmlexamples/C499_sweep
LGSynth91/cmlexamples/C499_synth
LGSynth91/cmlexamples/C880_orig
LGSynth91/cmlexamples/C880_sweep
LGSynth91/cmlexamples/C880_synth
LGSynth91/cmlexamples/C1355_orig
LGSynth91/cmlexamples/C1355_sweep
LGSynth91/cmlexamples/C1355_synth
LGSynth91/cmlexamples/C1908_orig
LGSynth91/cmlexamples/C1908_sweep
LGSynth91/cmlexamples/C1908_synth
LGSynth91/cmlexamples/C2670_orig
LGSynth91/cmlexamples/C2670_sweep
LGSynth91/cmlexamples/C2670_synth
LGSynth91/cmlexamples/C3540_orig
LGSynth91/cmlexamples/C3540_sweep
LGSynth91/cmlexamples/C3540_synth
LGSynth91/cmlexamples/C5315_orig
LGSynth91/cmlexamples/C5315_sweep
LGSynth91/cmlexamples/C5315_synth
LGSynth91/cmlexamples/C6288_orig
LGSynth91/cmlexamples/C6288_sweep
LGSynth91/cmlexamples/C6288_synth
LGSynth91/cmlexamples/C7552_orig
LGSynth91/cmlexamples/C7552_sweep
LGSynth91/cmlexamples/C7552_synth
LGSynth91/cmlexamples/cc_orig
LGSynth91/cmlexamples/cc_sweep
LGSynth91/cmlexamples/cc_synth
LGSynth91/cmlexamples/cht_orig
LGSynth91/cmlexamples/cht_sweep
LGSynth91/cmlexamples/cht_synth
LGSynth91/cmlexamples/cm42a_orig
LGSynth91/cmlexamples/cm42a_sweep
LGSynth91/cmlexamples/cm42a_synth
LGSynth91/cmlexamples/cm82a_orig
LGSynth91/cmlexamples/cm82a_sweep
LGSynth91/cmlexamples/cm82a_synth
LGSynth91/cmlexamples/cm85a_orig
LGSynth91/cmlexamples/cm85a_sweep
LGSynth91/cmlexamples/cm85a_synth
LGSynth91/cmlexamples/cm138a_orig
LGSynth91/cmlexamples/cm138a_sweep
LGSynth91/cmlexamples/cm138a_synth
LGSynth91/cmlexamples/cm150a_orig
LGSynth91/cmlexamples/cm150a_sweep
LGSynth91/cmlexamples/cm150a_synth
LGSynth91/cmlexamples/cm151a_orig
LGSynth91/cmlexamples/cm151a_sweep
LGSynth91/cmlexamples/cm151a_synth
LGSynth91/cmlexamples/cm152a_orig
LGSynth91/cmlexamples/cm152a_sweep
LGSynth91/cmlexamples/cm152a_synth
LGSynth91/cmlexamples/cm162a_orig
LGSynth91/cmlexamples/cm162a_sweep
LGSynth91/cmlexamples/cm162a_synth
LGSynth91/cmlexamples/cm163a_orig
LGSynth91/cmlexamples/cm163a_sweep
LGSynth91/cmlexamples/cm163a_synth
LGSynth91/cmlexamples/cmb_orig
LGSynth91/cmlexamples/cmb_sweep
LGSynth91/cmlexamples/cmb_synth
LGSynth91/cmlexamples/comp_orig
LGSynth91/cmlexamples/comp_sweep
LGSynth91/cmlexamples/comp_synth
LGSynth91/cmlexamples/cordic_orig
LGSynth91/cmlexamples/cordic_sweep
LGSynth91/cmlexamples/cordic_synth
LGSynth91/cmlexamples/count_orig
LGSynth91/cmlexamples/count_sweep
LGSynth91/cmlexamples/count_synth
LGSynth91/cmlexamples/cu_orig
LGSynth91/cmlexamples/cu_sweep
LGSynth91/cmlexamples/cu_synth
LGSynth91/cmlexamples/dalu_orig
LGSynth91/cmlexamples/dalu_sweep
LGSynth91/cmlexamples/dalu_synth
LGSynth91/cmlexamples/decod_orig
LGSynth91/cmlexamples/decod_sweep
LGSynth91/cmlexamples/decod_synth
LGSynth91/cmlexamples/des_orig
LGSynth91/cmlexamples/des_sweep
LGSynth91/cmlexamples/des_synth
LGSynth91/cmlexamples/example2_orig
LGSynth91/cmlexamples/example2_sweep
LGSynth91/cmlexamples/example2_synth
LGSynth91/cmlexamples/f51m_orig
LGSynth91/cmlexamples/f51m_sweep
LGSynth91/cmlexamples/f51m_synth
LGSynth91/cmlexamples/frg1_orig
LGSynth91/cmlexamples/frg1_sweep
LGSynth91/cmlexamples/frg1_synth
LGSynth91/cmlexamples/frg2_orig
LGSynth91/cmlexamples/frg2_sweep
LGSynth91/cmlexamples/frg2_synth
LGSynth91/cmlexamples/i1_orig
LGSynth91/cmlexamples/i1_sweep
LGSynth91/cmlexamples/i1_synth
LGSynth91/cmlexamples/i2_orig
LGSynth91/cmlexamples/i2_sweep
LGSynth91/cmlexamples/i2_synth
LGSynth91/cmlexamples/i3_orig
LGSynth91/cmlexamples/i3_sweep
LGSynth91/cmlexamples/i3_synth
LGSynth91/cmlexamples/i4_orig
LGSynth91/cmlexamples/i4_sweep
LGSynth91/cmlexamples/i5_orig
LGSynth91/cmlexamples/i5_sweep
LGSynth91/cmlexamples/i5_synth
LGSynth91/cmlexamples/i6_orig
LGSynth91/cmlexamples/i6_sweep
LGSynth91/cmlexamples/i6_synth
LGSynth91/cmlexamples/i7_orig
LGSynth91/cmlexamples/i7_sweep
LGSynth91/cmlexamples/i7_synth
LGSynth91/cmlexamples/i8_orig
LGSynth91/cmlexamples/i8_sweep
LGSynth91/cmlexamples/i8_synth
LGSynth91/cmlexamples/i9_orig
LGSynth91/cmlexamples/i9_sweep
LGSynth91/cmlexamples/i9_synth
LGSynth91/cmlexamples/i10_orig
LGSynth91/cmlexamples/i10_sweep
LGSynth91/cmlexamples/i10_synth
LGSynth91/cmlexamples/k2_orig
LGSynth91/cmlexamples/k2_sweep
LGSynth91/cmlexamples/k2_synth
LGSynth91/cmlexamples/lal_orig
LGSynth91/cmlexamples/lal_sweep
LGSynth91/cmlexamples/lal_synth
LGSynth91/cmlexamples/majority_orig
LGSynth91/cmlexamples/majority_sweep
LGSynth91/cmlexamples/majority_synth
LGSynth91/cmlexamples/mux_orig
LGSynth91/cmlexamples/mux_sweep
LGSynth91/cmlexamples/mux_synth
LGSynth91/cmlexamples/my_adder_orig
LGSynth91/cmlexamples/my_adder_sweep
LGSynth91/cmlexamples/my_adder_synth
LGSynth91/cmlexamples/pair_orig
LGSynth91/cmlexamples/pair_sweep
LGSynth91/cmlexamples/pair_synth
LGSynth91/cmlexamples/parity_orig
LGSynth91/cmlexamples/parity_sweep
LGSynth91/cmlexamples/parity_synth
LGSynth91/cmlexamples/pcle_orig
LGSynth91/cmlexamples/pcler8_orig
LGSynth91/cmlexamples/pcler8_sweep
LGSynth91/cmlexamples/pcler8_synth
LGSynth91/cmlexamples/pcle_sweep
LGSynth91/cmlexamples/pcle_synth
LGSynth91/cmlexamples/pm1_orig
LGSynth91/cmlexamples/pm1_sweep
LGSynth91/cmlexamples/pm1_synth
LGSynth91/cmlexamples/rot_orig
LGSynth91/cmlexamples/rot_sweep
LGSynth91/cmlexamples/rot_synth
LGSynth91/cmlexamples/sct_orig
LGSynth91/cmlexamples/sct_sweep
LGSynth91/cmlexamples/sct_synth
LGSynth91/cmlexamples/t481_orig
LGSynth91/cmlexamples/t481_sweep
LGSynth91/cmlexamples/t481_synth
LGSynth91/cmlexamples/tcon_orig
LGSynth91/cmlexamples/tcon_sweep
LGSynth91/cmlexamples/tcon_synth
LGSynth91/cmlexamples/term1_orig
LGSynth91/cmlexamples/term1_sweep
LGSynth91/cmlexamples/term1_synth
LGSynth91/cmlexamples/too_large_orig
LGSynth91/cmlexamples/too_large_sweep
LGSynth91/cmlexamples/ttt2_orig
LGSynth91/cmlexamples/ttt2_sweep
LGSynth91/cmlexamples/ttt2_synth
LGSynth91/cmlexamples/unreg_orig
LGSynth91/cmlexamples/unreg_sweep
LGSynth91/cmlexamples/unreg_synth
LGSynth91/cmlexamples/vda_orig
LGSynth91/cmlexamples/vda_sweep
LGSynth91/cmlexamples/vda_synth
LGSynth91/cmlexamples/x1_orig
LGSynth91/cmlexamples/x1_sweep
LGSynth91/cmlexamples/x1_synth
LGSynth91/cmlexamples/x2_orig
LGSynth91/cmlexamples/x2_sweep
LGSynth91/cmlexamples/x2_synth
LGSynth91/cmlexamples/x3_orig
LGSynth91/cmlexamples/x3_sweep
LGSynth91/cmlexamples/x3_synth
LGSynth91/cmlexamples/x4_orig
LGSynth91/cmlexamples/x4_sweep
LGSynth91/cmlexamples/x4_synth
LGSynth91/cmlexamples/z4ml_orig
LGSynth91/cmlexamples/z4ml_sweep
LGSynth91/cmlexamples/z4ml_synth
#
#
LGSynth91/smlexamples/bigkey_orig
LGSynth91/smlexamples/bigkey_sweep
LGSynth91/smlexamples/bigkey_synth
LGSynth91/smlexamples/clma_orig
LGSynth91/smlexamples/clma_sweep
LGSynth91/smlexamples/clmb_orig
LGSynth91/smlexamples/clmb_sweep
LGSynth91/smlexamples/clmb_synth
LGSynth91/smlexamples/dsip_orig
LGSynth91/smlexamples/dsip_sweep
LGSynth91/smlexamples/dsip_synth
LGSynth91/smlexamples/mm4a_orig
LGSynth91/smlexamples/mm4a_sweep
LGSynth91/smlexamples/mm4a_synth
LGSynth91/smlexamples/mm9a_orig
LGSynth91/smlexamples/mm9a_sweep
LGSynth91/smlexamples/mm9a_synth
LGSynth91/smlexamples/mm9b_orig
LGSynth91/smlexamples/mm9b_sweep
LGSynth91/smlexamples/mm9b_synth
LGSynth91/smlexamples/mm30a_orig
LGSynth91/smlexamples/mm30a_sweep
LGSynth91/smlexamples/mult16a_orig
LGSynth91/smlexamples/mult16a_sweep
LGSynth91/smlexamples/mult16a_synth
LGSynth91/smlexamples/mult16b_orig
LGSynth91/smlexamples/mult16b_sweep
LGSynth91/smlexamples/mult16b_synth
LGSynth91/smlexamples/mult32a_orig
LGSynth91/smlexamples/mult32a_sweep
LGSynth91/smlexamples/mult32a_synth
LGSynth91/smlexamples/mult32b_orig
LGSynth91/smlexamples/mult32b_sweep
LGSynth91/smlexamples/mult32b_synth
LGSynth91/smlexamples/s27_orig
LGSynth91/smlexamples/s27_sweep
LGSynth91/smlexamples/s27_synth
LGSynth91/smlexamples/s208.1_orig
LGSynth91/smlexamples/s208.1_sweep
LGSynth91/smlexamples/s208.1_synth
LGSynth91/smlexamples/s298_orig
LGSynth91/smlexamples/s298_sweep
LGSynth91/smlexamples/s298_synth
LGSynth91/smlexamples/s344_orig
LGSynth91/smlexamples/s344_sweep
LGSynth91/smlexamples/s344_synth
LGSynth91/smlexamples/s349_orig
LGSynth91/smlexamples/s349_sweep
LGSynth91/smlexamples/s349_synth
LGSynth91/smlexamples/s382_orig
LGSynth91/smlexamples/s382_sweep
LGSynth91/smlexamples/s382_synth
LGSynth91/smlexamples/s386_orig
LGSynth91/smlexamples/s386_sweep
LGSynth91/smlexamples/s386_synth
LGSynth91/smlexamples/s400_orig
LGSynth91/smlexamples/s400_sweep
LGSynth91/smlexamples/s400_synth
LGSynth91/smlexamples/s420.1_orig
LGSynth91/smlexamples/s420.1_sweep
LGSynth91/smlexamples/s420.1_synth
LGSynth91/smlexamples/s444_orig
LGSynth91/smlexamples/s444_sweep
LGSynth91/smlexamples/s444_synth
LGSynth91/smlexamples/s510_orig
LGSynth91/smlexamples/s510_sweep
LGSynth91/smlexamples/s510_synth
LGSynth91/smlexamples/s526n_orig
LGSynth91/smlexamples/s526n_sweep
LGSynth91/smlexamples/s526n_synth
LGSynth91/smlexamples/s526_orig
LGSynth91/smlexamples/s526_sweep
LGSynth91/smlexamples/s526_synth
LGSynth91/smlexamples/s641_orig
LGSynth91/smlexamples/s641_sweep
LGSynth91/smlexamples/s641_synth
LGSynth91/smlexamples/s713_orig
LGSynth91/smlexamples/s713_sweep
LGSynth91/smlexamples/s713_synth
LGSynth91/smlexamples/s820_orig
LGSynth91/smlexamples/s820_sweep
LGSynth91/smlexamples/s820_synth
LGSynth91/smlexamples/s832_orig
LGSynth91/smlexamples/s832_sweep
LGSynth91/smlexamples/s832_synth
LGSynth91/smlexamples/s838.1_orig
LGSynth91/smlexamples/s838.1_sweep
LGSynth91/smlexamples/s838.1_synth
LGSynth91/smlexamples/s953_orig
LGSynth91/smlexamples/s953_sweep
LGSynth91/smlexamples/s953_synth
LGSynth91/smlexamples/s1196_orig
LGSynth91/smlexamples/s1196_sweep
LGSynth91/smlexamples/s1196_synth
LGSynth91/smlexamples/s1423_orig
LGSynth91/smlexamples/s1423_sweep
LGSynth91/smlexamples/s1423_synth
LGSynth91/smlexamples/s1488_orig
LGSynth91/smlexamples/s1488_sweep
LGSynth91/smlexamples/s1488_synth
LGSynth91/smlexamples/s1494_orig
LGSynth91/smlexamples/s1494_sweep
LGSynth91/smlexamples/s1494_synth
LGSynth91/smlexamples/s5378_orig
LGSynth91/smlexamples/s5378_sweep
LGSynth91/smlexamples/s9234.1_orig
LGSynth91/smlexamples/s9234.1_sweep
LGSynth91/smlexamples/s9234.1_synth
LGSynth91/smlexamples/s13207.1_orig
LGSynth91/smlexamples/s13207.1_sweep
LGSynth91/smlexamples/s13207.1_synth
LGSynth91/smlexamples/s15850.1_orig
LGSynth91/smlexamples/s15850.1_sweep
LGSynth91/smlexamples/s35932_orig
LGSynth91/smlexamples/s35932_sweep
LGSynth91/smlexamples/s38417_orig
LGSynth91/smlexamples/s38417_sweep
LGSynth91/smlexamples/s38417_synth
LGSynth91/smlexamples/s38584.1_orig
LGSynth91/smlexamples/s38584.1_sweep
LGSynth91/smlexamples/sbc_orig
LGSynth91/smlexamples/sbc_sweep
LGSynth91/smlexamples/sbc_synth
#
#
texas/alu2_orig
texas/alu2_sweep
texas/alu2_synth
texas/a_orig
texas/a_sweep
texas/a_synth
texas/b1_orig
texas/b1_sweep
texas/b1_synth
texas/b9_orig
texas/b9_sweep
texas/b9_synth
texas/bbara_orig
texas/bbara_sweep
texas/bbara_synth
texas/bbsse_orig
texas/bbsse_sweep
texas/bbsse_synth
texas/bbtas_orig
texas/bbtas_sweep
texas/bbtas_synth
texas/beecount_orig
texas/beecount_sweep
texas/beecount_synth
texas/bigkey_orig
texas/bigkey_sweep
texas/bigkey_synth
texas/c8_orig
texas/c8_sweep
texas/c8_synth
texas/cbp_16_4_orig
texas/cbp_16_4_sweep
texas/cbp_16_4_synth
texas/cbp_32_4_orig
texas/cbp_32_4_sweep
texas/cbp_32_4_synth
texas/cc_orig
texas/cc_sweep
texas/cc_synth
texas/cht_orig
texas/cht_sweep
texas/cht_synth
texas/clma_orig
texas/clma_sweep
texas/clmb_orig
texas/clmb_sweep
texas/clmb_synth
texas/cm42a_orig
texas/cm42a_sweep
texas/cm42a_synth
texas/cm82a_orig
texas/cm82a_sweep
texas/cm82a_synth
texas/cm85a_orig
texas/cm85a_sweep
texas/cm85a_synth
texas/cm138a_orig
texas/cm138a_sweep
texas/cm138a_synth
texas/cm150a_orig
texas/cm150a_sweep
texas/cm150a_synth
texas/cm151a_orig
texas/cm151a_sweep
texas/cm151a_synth
texas/cm152a_orig
texas/cm152a_sweep
texas/cm152a_synth
texas/cm162a_orig
texas/cm162a_sweep
texas/cm162a_synth
texas/cm163a_orig
texas/cm163a_sweep
texas/cm163a_synth
texas/cmb_orig
texas/cmb_sweep
texas/cmb_synth
texas/comp_orig
texas/comp_sweep
texas/comp_synth
texas/cordic_latches_orig
texas/cordic_latches_sweep
texas/cordic_latches_synth
texas/cse_orig
texas/cse_sweep
texas/cse_synth
texas/cu_orig
texas/cu_sweep
texas/cu_synth
texas/des_orig
texas/des_sweep
texas/des_synth
texas/dk14_orig
texas/dk14_sweep
texas/dk14_synth
texas/dk15_orig
texas/dk15_sweep
texas/dk15_synth
texas/dk17_orig
texas/dk17_sweep
texas/dk17_synth
texas/dk27_orig
texas/dk27_sweep
texas/dk27_synth
texas/dk512_orig
texas/dk512_sweep
texas/dk512_synth
texas/donfile_orig
texas/donfile_sweep
texas/donfile_synth
texas/dsip_orig
texas/dsip_sweep
texas/dsip_synth
texas/ex1_orig
texas/ex1_sweep
texas/ex1_synth
texas/ex2_orig
texas/ex2_sweep
texas/ex2_synth
texas/ex3_orig
texas/ex3_sweep
texas/ex3_synth
texas/ex4_orig
texas/ex4_sweep
texas/ex4_synth
texas/ex5_orig
texas/ex5_sweep
texas/ex5_synth
texas/ex6_orig
texas/ex6_sweep
texas/ex6_synth
texas/ex7_orig
texas/ex7_sweep
texas/ex7_synth
texas/example2_orig
texas/example2_sweep
texas/example2_synth
texas/frg1_orig
texas/frg1_sweep
texas/frg1_synth
texas/frg2_orig
texas/frg2_sweep
texas/frg2_synth
texas/i1_orig
texas/i1_sweep
texas/i1_synth
texas/i2_orig
texas/i2_sweep
texas/i2_synth
texas/i3_orig
texas/i3_sweep
texas/i3_synth
texas/i4_orig
texas/i4_sweep
texas/i5_orig
texas/i5_sweep
texas/i5_synth
texas/i6_orig
texas/i6_sweep
texas/i6_synth
texas/i7_orig
texas/i7_sweep
texas/i7_synth
texas/i8_orig
texas/i8_sweep
texas/i8_synth
texas/i9_orig
texas/i9_sweep
texas/i9_synth
texas/i10_orig
texas/i10_sweep
texas/i10_synth
texas/k2_orig
texas/k2_sweep
texas/k2_synth
texas/keyb_orig
texas/keyb_sweep
texas/keyb_synth
texas/key_orig
texas/key_sweep
texas/key_synth
texas/kirkman_orig
texas/kirkman_sweep
texas/kirkman_synth
texas/lal_orig
texas/lal_sweep
texas/lal_synth
texas/lion9_orig
texas/lion9_sweep
texas/lion9_synth
texas/lion_orig
texas/lion_sweep
texas/lion_synth
texas/majority_orig
texas/majority_sweep
texas/majority_synth
texas/mark1_orig
texas/mark1_sweep
texas/mark1_synth
texas/mclc_orig
texas/mclc_sweep
texas/mclc_synth
texas/mc_orig
texas/mc_sweep
texas/mc_synth
texas/minmax2_orig
texas/minmax2_sweep
texas/minmax2_synth
texas/minmax5_orig
texas/minmax5_sweep
texas/minmax5_synth
texas/minmax10_orig
texas/minmax10_sweep
texas/minmax10_synth
texas/minmax12_orig
texas/minmax12_sweep
texas/minmax12_synth
texas/minmax20_orig
texas/minmax20_sweep
texas/minmax20_synth
texas/minmax32_orig
texas/minmax32_sweep
texas/mm4a_orig
texas/mm4a_sweep
texas/mm4a_synth
texas/mm9a_orig
texas/mm9a_sweep
texas/mm9a_synth
texas/mm9b_orig
texas/mm9b_sweep
texas/mm9b_synth
texas/mm30a_orig
texas/mm30a_sweep
texas/modulo12_orig
texas/modulo12_sweep
texas/modulo12_synth
texas/mult16a_orig
texas/mult16a_sweep
texas/mult16a_synth
texas/mult16b_orig
texas/mult16b_sweep
texas/mult16b_synth
texas/mult32a_orig
texas/mult32a_sweep
texas/mult32a_synth
texas/mult32b_orig
texas/mult32b_sweep
texas/mult32b_synth
texas/mult32_orig
texas/mult32_sweep
texas/mult32_synth
texas/mux_orig
texas/mux_sweep
texas/mux_synth
texas/my_adder_orig
texas/my_adder_sweep
texas/my_adder_synth
texas/opus_orig
texas/opus_sweep
texas/opus_synth
texas/pair_orig
texas/pair_sweep
texas/pair_synth
texas/parity_orig
texas/parity_sweep
texas/parity_synth
texas/pcle_orig
texas/pcler8_orig
texas/pcler8_sweep
texas/pcler8_synth
texas/pcle_sweep
texas/pcle_synth
texas/planet1_orig
texas/planet1_sweep
texas/planet1_synth
texas/planet_orig
texas/planet_sweep
texas/planet_synth
texas/pm1_orig
texas/pm1_sweep
texas/pm1_synth
texas/rot_orig
texas/rot_sweep
texas/rot_synth
texas/s1a_orig
texas/s1a_sweep
texas/s1a_synth
texas/s1_orig
texas/s1_sweep
texas/s1_synth
texas/s8_orig
texas/s8_sweep
texas/s8_synth
texas/s27_orig
texas/s27_sweep
texas/s27_synth
texas/s208.1_orig
texas/s208.1_sweep
texas/s208.1_synth
texas/s298_orig
texas/s298_sweep
texas/s298_synth
texas/s344_orig
texas/s344_sweep
texas/s344_synth
texas/s349_orig
texas/s349_sweep
texas/s349_synth
texas/s382_orig
texas/s382_sweep
texas/s382_synth
texas/s386_orig
texas/s386_sweep
texas/s386_synth
texas/s400_orig
texas/s400_sweep
texas/s400_synth
texas/s420.1_orig
texas/s420.1_sweep
texas/s420.1_synth
texas/s444_orig
texas/s444_sweep
texas/s444_synth
texas/s510_orig
texas/s510_sweep
texas/s510_synth
texas/s526n_orig
texas/s526n-simp_orig
texas/s526n-simp_sweep
texas/s526n-simp_synth
texas/s526ns_orig
texas/s526ns_sweep
texas/s526ns_synth
texas/s526n_sweep
texas/s526n_synth
texas/s526_orig
texas/s526_sweep
texas/s526_synth
texas/s641_orig
texas/s641-retime_orig
texas/s641-retime_sweep
texas/s641-retime_synth
texas/s641-r_orig
texas/s641-r_sweep
texas/s641-r_synth
texas/s641_sweep
texas/s641_synth
texas/s713_orig
texas/s713s_orig
texas/s713s_sweep
texas/s713s_synth
texas/s713_sweep
texas/s713_synth
texas/s820_orig
texas/s820_sweep
texas/s820_synth
texas/s832_orig
texas/s832_sweep
texas/s832_synth
texas/s838.1_orig
texas/s838.1_sweep
texas/s838.1_synth
texas/s953n_orig
texas/s953n_sweep
texas/s953n_synth
texas/s953_orig
texas/s953s_orig
texas/s953s_sweep
texas/s953s_synth
texas/s953_sweep
texas/s953_synth
texas/s1196_orig
texas/s1196_sweep
texas/s1196_synth
texas/s1238_orig
texas/s1238_sweep
texas/s1238_synth
texas/s1423_orig
texas/s1423_sweep
texas/s1423_synth
texas/s1488_orig
texas/s1488_sweep
texas/s1488_synth
texas/s1494_orig
texas/s1494_sweep
texas/s1494_synth
texas/s5378_orig
texas/s5378_sweep
texas/s9234.1_orig
texas/s9234.1_sweep
texas/s9234.1_synth
texas/s9234s_orig
texas/s9234s_sweep
texas/s9234s_synth
texas/s13207.1_orig
texas/s13207.1_sweep
texas/s13207.1_synth
texas/s13207s_orig
texas/s13207s_sweep
texas/s15850.1_orig
texas/s15850.1_sweep
texas/s15850s_orig
texas/s15850s_sweep
texas/s35932_orig
texas/s35932s_orig
texas/s35932s_sweep
texas/s35932_sweep
texas/s38417_orig
texas/s38417s_orig
texas/s38417s_sweep
texas/s38417s_synth
texas/s38417_sweep
texas/s38417_synth
texas/s38584.1_orig
texas/s38584.1_sweep
texas/sand_orig
texas/sand_sweep
texas/sand_synth
texas/sbc_orig
texas/sbc_sweep
texas/sbc_synth
texas/scf_orig
texas/scf_sweep
texas/scf_synth
texas/sct_orig
texas/sct_sweep
texas/sct_synth
texas/shiftreg_orig
texas/shiftreg_sweep
texas/shiftreg_synth
texas/sse_orig
texas/sse_sweep
texas/sse_synth
texas/styr_orig
texas/styr_sweep
texas/styr_synth
texas/t481_orig
texas/t481_sweep
texas/t481_synth
texas/tav_orig
texas/tav_sweep
texas/tav_synth
texas/tbk_orig
texas/tbk-retime_orig
texas/tbk-retime_sweep
texas/tbk-retime_synth
texas/tbk_sweep
texas/tbk_synth
texas/tcon_orig
texas/tcon_sweep
texas/tcon_synth
texas/term1_orig
texas/term1_sweep
texas/term1_synth
texas/too_large_orig
texas/too_large_sweep
texas/traffic_orig
texas/traffic_sweep
texas/traffic_synth
texas/train4_orig
texas/train4_sweep
texas/train4_synth
texas/ttt2_orig
texas/ttt2_sweep
texas/ttt2_synth
texas/vda_orig
texas/vda_sweep
texas/vda_synth
texas/x1_orig
texas/x1_sweep
texas/x1_synth
texas/x2_orig
texas/x2_sweep
texas/x2_synth
texas/x3_orig
texas/x3_sweep
texas/x3_synth
texas/x4_orig
texas/x4_sweep
texas/x4_synth
texas/z4ml_orig
texas/z4ml_sweep
texas/z4ml_synth
)

echo "=============================="
echo "Downloading the benchmarks ..."

if [ ! -e "iwls_1.0.tar.gz" ];
then
  wget http://www.eecs.berkeley.edu/~alanmi/benchmarks/iwls2002/iwls_1.0.tar.gz
fi

if [ ! -d "iwls_1.0" ];
then
  tar -xzf iwls_1.0.tar.gz
fi

echo "=============================="
echo "Converting the benchmarks ..."
rm -rf $TARGET # just to clean up
for filename in "${IN_FILES[@]}"
do
  mkdir -p `dirname $TARGET$filename`
  $ABC -c "read_verilog $SRC$filename.v; strash; zero; write_aiger $TARGET$filename.aig"
done

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
