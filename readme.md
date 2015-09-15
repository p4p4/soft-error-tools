# TOC

# AddParityTool
in AddParityTool/

adds some protection-logic with the additional alarm output to a given circuit. The protection logic
simply compares the parity-sum of the latch-inputs from the previous state with the parity sum of the
latch-outputs from the current state.

USAGE: ./addParityTool <aiger-input> <percentage> <avg-latches> <aiger-output>
  WHERE
	 <aiger-input>.......path to the aiger input file. The mode used is ASCII for a '.aag'
	                     suffix and binary mode otherwise.
	 <percentage>........percentage of latches to protect
	 <avg-latches>.......average nuber of latches to protect with one additional parity latch
	 <aiger-output>......path to the aiger input file. The mode used is ASCII for a '.aag'
	                     suffix and binary mode otherwise.

# ImmortalTool  TODO rename to openSEA
in ImmortalTool/



# AlarmToMC 
in AlarmToMC/

converts a circuit with protection logic to a model-checking problem. The bad-signal of the resulting
circuit is raised whenever it is possible to flip some latch-output which changes the outputs of the
orignal circuit but without raising the alarm-output.

USAGE: ./alarmToMC <input-aiger-file> <output-aiger-file>
     <input-aiger-file> ..... path to the aiger circuit with protection logic
     <output-aiger-file> .... path to the resulting MC-compatible aiger circuit

The resulting circuit can be used as input for any model checker (e.g. BLIMC, IC3)

