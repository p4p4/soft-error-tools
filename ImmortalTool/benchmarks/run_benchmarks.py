#!/usr/bin/python
import subprocess
import sys

immortal_binary = "./immortal-bin"
immortal_backend = "-b stla -m 0"

add_parity_binary = "./../../AddParityTool/addParityTool"
benchmark_directory = "/home/pklampfl/soft-error-analysis/AddParityTool/benchmarks/"
list_of_benchmarks = "basic_benchmarks.txt"
percentage_to_protect = "50"
avg_latches = "2"

with open(list_of_benchmarks) as f:
    for aiger_path in f:
        aiger_path = aiger_path.rstrip()
        print "\n================================================================================"
        out_file = "tmp/protected_"+aiger_path.replace("/","_")
        cmd_add_parity = add_parity_binary + " " + benchmark_directory + aiger_path + " " + percentage_to_protect + " " + avg_latches + " " + out_file

        return_code = subprocess.call(cmd_add_parity, shell=True, stdout=subprocess.PIPE)
        if return_code != 0:
            print "Error calling the addParityTool. CMD: " + cmd_add_parity
            sys.exit(0)
        sys.stdout.flush()

        cmd_immortal = immortal_binary + " -i " + out_file + " -tcr 3 15 " + immortal_backend +" --print=L"
        p = subprocess.Popen(cmd_immortal, shell=True, stderr=subprocess.PIPE) # run
        while True: # print results
            out = p.stderr.read(1)
            if out == '' and p.poll() != None:
                break
            if out != '':
                sys.stdout.write(out)
                sys.stdout.flush()


