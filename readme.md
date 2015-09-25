# TOC:
  1. AddParityTool
  2. openSEA
     - How to install
     - How to use
     - Information for developers
  3. AlarmToMC
  4. other Folders



# 1 AddParityTool
in AddParityTool/

adds a protection-logic with the additional alarm output to a given circuit. The protection logic
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



# 2 openSEA
in ImmortalTool/    # TODO: rename folder name!

## 2.1 How to install:
1. create a folder where you want to install the third-party libraries and set the path to it in 
   the environment variable IMMORTALTP
2. run ImmortalTool/ext_tools/install_all.sh
3. run "make" command in ImmortalTool/ to compile the program.

* NOTE: if you have installed DEMIURGE on your computer, it suffices to set the environment variable
IMMORTALTP to the same path as DEMIURGETPD instead of re-installing the libraries as in 1. and 2.

the *executable* is located here: ImmortalTool/build/src/immortal-bin



## 2.2 How to use openSEA:
openSEA tries to find soft-error vulnerabilities in a given circuit

 * this is basically the same information as the output of ./immortal-bin -h :

        Usage: immortal-bin [options]

        Options:
        -h, --help
                     Show this help message and exit.
        -v, --version
                     Print version information and exit.
        -i INPUT_FILE, --in=INPUT_FILE
                     The AIGER file with additional alarm output
                     It can be:
                      - a binary AIGER file (header starts with 'aig'),
                      - an ASCII AIGER file (header starts with 'aag'),
                      - a compressed file (INPUT_FILE ends with '.gz').
        -tc TESTCASE_FILE(s)
                    The TestCase(s) to use. A TESTCASE_FILE contains a list 
                    of input-vectors. One input-vector contains values for 
                    each input to a given time-step.
                    One line of such a file represents one time-step,
                    containing '0's or '1's as input values. Some modes also
                    allow to leave inputs open(use '?' instead of constants)
        -tcr NUM_TCs NUM_TIMESTEPS
                    can be used instead of -tc.
                    randomly generates NUM_TCs test-cases, each of them have
                    a length of NUM_TIMESTEPS time-steps
        -mc NUM_TIMESTEPS
                    can be used instead of -tc for modes, which support free
                    input values. Generates a TestCase for the given length 
                    with all input values open. (similar to model-checking) 
        -d [PATH] 
                     Creates a diagnostic output, called ErrorTraces,
                     which contains detailed information on how to reproduce
                     an undetected soft-error.
                     If -d is followed by a PATH, the diagnostic output gets
                     written as text-file to that location, otherwise it is 
                     printed to stdout.
        -b BACKEND, --backend=BACKEND
                     The back-end to be used for detecting vulnerabilities.
                     Different back-ends implement different algorithms.
                     The following back-ends are available:
                     sim:  The simulation-bases analysis 
                             Can only handle concrete TestCase values 
                     sta:  The symbolic-time analysis
                             SAT-Solver based algorithm. The point in time
                             when to introduce a flip is symbolic, but the
                             location (the latch) is fixed. 
                     stla: The symbolic-time-location analysis
                             SAT-Solver based algorithm. The point in time
                             when to introduce a flip is symbolic, the    
                             location (the latch to flip) is  also 
                             symbolic.  
                     The default is 'sim'.
        -m MODE , --mode=MODE
                     Some back-ends can be used in several modes (certain
                     optimizations enabled or disabled, etc.).
                     MODE is an integer number to select one such mode. 
                     The following modes are supported: 
                     Back-end 'sim': 
                       0: standard mode
                     Back-end 'sta': 
                       0: NAIVE mode - always copy whole transition relation
                          when unrolling it.
                       1: SYMBOLIC_SIMULATION - perform symbolic simulation
                          and generate the unrolled transition relation 
                          on the fly 
                       2: FREE_INPUTS - as the previous method, but allows
                          to leave some or all values in the given TestCase
                          to be left open (write '?' instead of '0' or '1')
                     Back-end 'stla': 
                       0: STANDARD - perform symbolic simulation
                          and generate the unrolled transition relation 
                          on the fly 
                       1: FREE_INPUTS - as the previous method, but allows
                          to leave some or all values in the given TestCase
                          to be left open (write '?' instead of '0' or '1')
                     The default is 0.
        -p PRINT, --print=PRINT
                     A string indicating which messages to print. Every
                     character activates a certain type of message. The
                     order and case of the characters does not matter.
                     Possible characters are:
                     E:      Enables the printing of error messages.
                     W:      Enables the printing of warnings.
                     R:      Enables the printing of results.
                     D:      Enables the printing of debugging messages.
                     I:      Enables the printing of extra information
                             (such as progress information).
                     L:      Enables the printing of statistics
                             (such as performance measures).
                     The default is 'EWRIL'.
        -s SAT_SOLVER, --sat_sv=SAT_SOLVER
                     The SAT solver to use.
                     The following SAT solvers are available:
                     lin_api: Uses the Lingeling solver via its API.
                     min_api: Uses the MiniSat solver via its API.
                     pic_api: Uses the PicoSat solver via its API.
                     The default is 'min_api'.
        Have fun!




## 2.3 Information for Developers


# 3 AlarmToMC 
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

# 4 other Folders

## benchmark_files/ 
- contains two scripts to download benchmarks and convert them to AIGER
- these resulting circuits don't have any protection logic, you may use AddParityTool from 1.)
- the script to convert the benchmarks needs abc installed, set the path accordingly

## presentation/
- A presentation on soft-error-analysis which explains some principles of the tools within this repository
- held in September 2015 by Patrick Klampfl

