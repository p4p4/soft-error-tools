# TOC:
  1. OpenSEA
     - How to install
     - How to use
  2. AddParityTool
  3. AlarmToMC
  4. other Folders


# 1 OpenSEA
in OpenSEA/

OpenSEA can analyze circuits in [AIGER](http://fmv.jku.at/aiger/FORMAT.aiger) file-format
with included error-detection or error-correction logic. The last output of the circuit has to be the *alarm*-output, which is raised if an error is *detected*. Set the *alarm*-output to constant `0` if only error-*correction* is used.

OpenSEA implements algorithms to detect *Vulnerable Latches*,
*False Positives* and *Definitely Protected Latches*

## 1.1 How to install:
### Dependencies (tested for Ubuntu 16.10):
`zlib1g-dev cmake build-essential git`

### Dependencies (tested for CentOS/RHEL 6.5)
`yum install -y gcc gcc-c++ make cmake zlib-devel git glibc-static zlib-static`

Quick guide:
1. create a folder for third-party libraries and store the path in an environment variable `$IMMORTALTP`

    * create folder `mkdir /path/to/libs/`

    * append to  `.bashrc`: `export IMMORTALTP=/path/to/libs`

2. run `sh OpenSEA/ext_tools/install_all.sh` to install all necessary libraries

3. in folder *OpenSEA/* run `make` to compile the program.

    the *executable* is located in `OpenSEA/build/src/immortal-bin`

for more details, see `OpenSEA/HowToCompile.md`


## 1.2 How to use OpenSEA:
OpenSEA analyzes the quality of soft-error protection logic in circuits. It can detect vulnerable components, false-positives and definitely protected components.

 * this is basically the same information as the output of ./immortal-bin -h :

```
Usage: immortal-bin [options]

Options:
  -h, --help
                 Show this help message and exit.
  -v, --version
                 Print version information and exit.
  -i INPUT_FILE, --in=INPUT_FILE
                 The AIGER circuit with additional alarm output
                 It can be:
                  - a binary AIGER file (header starts with 'aig'),
                  - an ASCII AIGER file (header starts with 'aag'),
                  - a compressed file (INPUT_FILE ends with '.gz').
  -env ENVIRONMENT_FILE
                 An *optional* environment circuit in aiger representation
                 which defines relevance of output values and optionally
                 the allowed input combinations.
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
                         Finds vulnerabilities using a test-case
                 sta:  The symbolic-time analysis
                         Finds vulnerabilities using a test-case
                         SAT-Solver based algorithm. The point in time
                         when to introduce a flip is symbolic, but the
                         location (the latch) is fixed. 
                 stla: The symbolic-time-location analysis
                         Finds vulnerabilities using a test-case
                         SAT-Solver based algorithm. The point in time
                         when to introduce a flip is symbolic, the    
                         location (the latch to flip) is  also 
                         symbolic.  
                 fp:   The false-positives analysis
                         Finds false positives using a test-case
                         Adoptions of the sta and stla algorithms to
                         find false positives (instead of vulnerabilities)
                 bdd:  The Binary Decision Diagram analysis
                         Finds vulnerabilities using a test-case
                         Similar to 'sta' and 'stla', but using BDDs
                         instead of SAT-solvers
                 dp:   The Definitely Protected latches analysis
                         Finds definitely protected latches
                 The default is 'sim'.
  -m MODE , --mode=MODE
                 Some back-ends can be used in several modes (certain
                 optimizations enabled or disabled, etc.).
                 MODE is an integer number to select one such mode. 
                 The following modes are supported: 
                 Back-end 'sim': 
                   0: STANDARD mode
                   1: FREE_INPUTS - as the previous method, but allows
                      to leave some or all values in the given TestCase
                      open (write '?' instead of '0' or '1')
                 Back-end 'sta': 
                   0: NAIVE mode - always copy whole transition relation
                      when unrolling it.
                   1: SYMBOLIC_SIMULATION - perform symbolic simulation
                      and generate the unrolled transition relation 
                      on the fly 
                   2: FREE_INPUTS - as the previous method, but allows
                      to leave some or all values in the given TestCase
                      open (write '?' instead of '0' or '1')
                 Back-end 'stla': 
                   0: STANDARD - perform symbolic simulation
                      and generate the unrolled transition relation 
                      on the fly 
                   1: FREE_INPUTS - as the previous method, but allows
                      to leave some or all values in the given TestCase
                      open (write '?' instead of '0' or '1')
                 Back-end 'fp': 
                   0: SYMB_TIME mode - find false positives with an algorithm
                      working similar to the one of the 'sta' backend.
                   1: SYMB_TIME_LOCATION - find false positives with an algorithm
                      working similar to the one of the 'stla' backend.
                   2: SYMB_TIME_INPUTS - as mode 0, but allows
                      to leave some or all values in the given TestCase
                      open (write '?' instead of '0' or '1')
                   3: SYMB_TIME_LOCATION_INPUTS - as mode 1, but allows
                      to leave some or all values in the given TestCase
                      open (write '?' instead of '0' or '1')
                 Back-end 'bdd': find vulnerable latches using BDDs (uses CUDD)
                   3: concrete input values only
                   4: capable of free input values
                 Back-end 'dp': find definitely protected latches
                   1: testing latches individually for 1-step protection
                   2: testing latches simultaneously for 1-step protection
                   3: testing latches individually for k-step protection
                   4: testing latches simultaneously for k-step protection
                   [-k <k> <init>] <init> = number of overapproximation steps
                                   <k> = the maximum number of steps to recover
                 The default is 0.
  -e FILE, --exclude=FILE
                 excludes the latches listed in FILE from the analysis.
  -r FILE, --results=FILE
                 stores the latches detected by the algorithm to FILE.
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
```


# 2 AddParityTool
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

## doc/
### doc/presentation/
- A presentation on soft-error-analysis which explains some principles of the tools within this repository
- held in September 2015 by Patrick Klampfl

### doc/paper/
- An article about the OpenSEA implementation and experimental results
- written in February 2016 by Patrick Klampfl

