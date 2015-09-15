
# -------------------------------------------------------------------------------------------------
# TOC:
  1. AddParityTool
  2. openSEA
     - how to install
  3. AlarmToMC
# -------------------------------------------------------------------------------------------------



# -------------------------------------------------------------------------------------------------#
# 1 AddParityTool                                                                                  #
# -------------------------------------------------------------------------------------------------#
in AddParityTool/

adds some protection-logic with the additional alarm output to a given circuit. The protection logic
simply compares the parity-sum of the latch-inputs from the previous state with the parity sum of the
latch-outputs from the current state. The tool has the ability to specify how many latches should be
protected and how many additional latches should be used therefore.

USAGE: ./addParityTool <aiger-input> <percentage> <avg-latches> <aiger-output>
  WHERE
	 <aiger-input>.......path to the aiger input file. The mode used is ASCII for a '.aag'
	                     suffix and binary mode otherwise.
	 <percentage>........percentage of latches to protect
	 <avg-latches>.......average nuber of latches to protect with one additional parity latch
	 <aiger-output>......path to the aiger input file. The mode used is ASCII for a '.aag'
	                     suffix and binary mode otherwise.



# -------------------------------------------------------------------------------------------------#
# 2 openSEA                                                                                        #
# -------------------------------------------------------------------------------------------------#
in ImmortalTool/    # TODO: rename!





# -------------------------------------------------------------------------------------------------#
# 3 AlarmToMC                                                                                      #
# -------------------------------------------------------------------------------------------------#
in AlarmToMC/

converts a circuit with protection logic to a model-checking problem. The bad-signal of the resulting
circuit is raised whenever it is possible to flip some latch-output which changes the outputs of the
orignal circuit but without raising the alarm-output.

USAGE: ./alarmToMC <input-aiger-file> <output-aiger-file>
     <input-aiger-file> ..... path to the aiger circuit with protection logic
     <output-aiger-file> .... path to the resulting MC-compatible aiger circuit

// @brief creates 2 copies of original_circuit to translate it to a MC problem:
// error if the outputs of the 2 circuits are different, but alarm is set to false
//
//					                   +---------+ outputs1:
//					i1  +-----+--------+         +-------+
//					i2  |-----|--+-----| circuit |       |
//					i3  +-----|--|---+-+         +----+  |    +-------+
//				           	  |  |   | +---------+    |  +----+       |
//          				  |  |   |                +-------+       |          +----+
//	          				  |  |   |             outputs2:  |       +----------+    |
//				          	  |  |   | +---------+    +-------+  !=   |          |AND +-----+ bad
//	           				  |  |   +-+         +----+       |       |     +---o|    |
//		           			  |  +-----| circ_cpy|       +----+       |     |    |    |
//		          			  +--------| (modif) +-------+    |       |     |    +----+
//			 	   	f_in +----[magic]--+-+-------+-----+      +-------+     |
//			   	                         |             |                    |
//	  		       ci .. cn -------------+             +------------[magic]-+
//													                       alarm_output
//
// f_in [magic] : the real f signal is only true when f_in is set to true the first time
// alarm_output [magic] : once the alarm output is raised it stays true forever.


The resulting circuit can be used as input for any model checker (e.g. BLIMC, IC3)

