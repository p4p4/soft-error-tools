#!/usr/bin/python
import subprocess
import sys
import time
#############################################################################################
##                                                                                         ##
##   This script takes a list of circuits (without protection circuit)                     ##
##   and converts them using AddParityTool so that they have an alarm signal.              ##
##   The converted circuits are then used as input for AlarmToMC, which converts the       ##
##   circuit to a MC problem. That in turn gets checked with the MC BLIMC                  ##
##                                                                                         ##
############################################################################################# 


# set benchmarks
BENCHMARK_DIR = "../../benchmark_files/"        # benchmarking circuits without protection circuits are in here
# list of circuit-filenames located within the BENCHMARK_DIR:
BENCHMARKS_LIST = "all_benchmarks.txt"	        # all benchmarks



# convert input circuits with addParityTool (protects the circuit with additional latches, adds an alarm-output)
add_parity_binary = "./../../AddParityTool/addParityTool"
percentage_to_protect = "99"    # randomly select this the given percentage of latches to protect 
avg_latches = "2" 		        # one additional error_latch protects 'avg_latches' latches

TMP_DIR = "tmp/"

# for each benchmark
with open(BENCHMARKS_LIST) as f:
    for aiger_path in f:
        aiger_path = aiger_path.rstrip()
        print "\n================================================================================"
        prot_file = "prot_"+aiger_path.replace("/","_")
        out_file = TMP_DIR + prot_file
        cmd_add_parity = add_parity_binary + " " + BENCHMARK_DIR + aiger_path + " " + percentage_to_protect + " " + avg_latches + " " + out_file

	# convert benchmark using addParityTool
        print "input: " + aiger_path
        return_code = subprocess.call(cmd_add_parity, shell=True, stdout=subprocess.PIPE)
        if return_code != 0:
            print "Error calling the addParityTool. CMD: " + cmd_add_parity
            sys.exit(0)
        sys.stdout.flush()
        print "protected: " + prot_file

        print "Inputs: X, Latches: Y, Error Latches: Z, Outputs: A" # todo: read values
        

	# convert to MC problem
        mc_file = TMP_DIR + "mc_"+aiger_path.replace("/","_")
        cmd_mc_tool = "./../alarmToMC " + out_file + " " + mc_file
        return_code2 = subprocess.call(cmd_mc_tool, shell=True, stdout=subprocess.PIPE)
        if return_code2 != 0:
            print "Error calling the alarmToMC CMD: " + cmd_mc_tool
            sys.exit(0)
        sys.stdout.flush()

        print "BackEnd: BLIMC, input: " + "mc_"+aiger_path.replace("/","_")

	# execute model-checker
        maxk = "15"
        cmd_mc_tool = "./../../../libs/blimc/blimc " + maxk + " " + mc_file

        start = time.time()
        return_code3 = subprocess.call(cmd_mc_tool, shell=True, stdout=subprocess.PIPE)
        end = time.time()
        duration = end - start

        #print cmd_mc_tool
        if return_code3 == 10:
            print "#Vulnerabilities found: 1"
        elif return_code3 == 20:
            print "#Vulnerabilities found: 0"
        else:
            print "strange BLIMC ret code: " + str(return_code3)
            sys.exit(0)

        print "Overall execution time: "+str(duration)+" sec CPU time, "+str(duration)+" sec real time."

        sys.stdout.flush()




