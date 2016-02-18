#!/usr/bin/python
import subprocess
import sys
#############################################################################################
##                                                                                         ##
##   This script takes a list of circuits (without protection circuit)                     ##
##   and converts them using AddParityTool so that they have an alarm signal.              ##
##   The converted circuits are then used as input for the ImmortalTool, which             ##
##   tries to search for vulnerable latches.                                               ##
##                                                                                         ##
############################################################################################# 

# select tool + mode
IMMORTAL_BIN = "./immortal-bin"
BACKEND_MODE = "-b sta -m 0"

NUM_RAND_TC = "3"
RAND_TC_LEN = "15"

# set benchmarks
BENCHMARK_DIR = "../../benchmark_files/"        # benchmarking circuits without protection circuits are in here
# list of circuit-filenames located within the BENCHMARK_DIR:
BENCHMARKS_LIST = "all_benchmarks.txt"
#BENCHMARKS_LIST = "standard_benchmarks.txt"	# easy benchmarks
#BENCHMARKS_LIST = "medium_benchmarks.txt"	    # slightly more challenging
#BENCHMARKS_LIST = "hard_benchmarks_1.txt"	    # 200 - 800 latches


# convert input circuits with addParityTool (protects the circuit with additional latches, adds an alarm-output)
add_parity_binary = "./../../AddParityTool/addParityTool"
percentage_to_protect = "90"    # randomly select this the given percentage of latches to protect 
avg_latches = "2" 		        # one additional error_latch protects 'avg_latches' latches

TMP_DIR = "tmp/sta090"

# for each benchmark
with open(BENCHMARKS_LIST) as f:
    for aiger_path in f:
        aiger_path = aiger_path.rstrip()
        print "\n================================================================================"
        out_file = TMP_DIR + "protected_"+aiger_path.replace("/","_")
        cmd_add_parity = add_parity_binary + " " + BENCHMARK_DIR + aiger_path + " " + percentage_to_protect + " " + avg_latches + " " + out_file

	# convert benchmark using addParityTool
        return_code = subprocess.call(cmd_add_parity, shell=True, stdout=subprocess.PIPE)
        if return_code != 0:
            print "Error calling the addParityTool. CMD: " + cmd_add_parity
            sys.exit(0)
        sys.stdout.flush()

	# execute converted benchmark with random inputs.
        cmd_immortal = IMMORTAL_BIN + " -i " + out_file + " -tcr 3 15 " + BACKEND_MODE +" --print=L --seed=123456"
        p = subprocess.Popen(cmd_immortal, shell=True, stderr=subprocess.PIPE) 
        while True: # print output of tool
            out = p.stderr.read(1)
            if out == '' and p.poll() != None:
                break
            if out != '':
                sys.stdout.write(out)
                sys.stdout.flush()


