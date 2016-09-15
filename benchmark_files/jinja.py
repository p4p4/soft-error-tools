#
# Author: Patrick Klampfl
# Date: September 12th, 2016
#
# vl2mv doesn't seem to support generate-loops, which is why I use the 
# jinja templating engine instead. More information at: jinja.pocoo.org
#

from jinja2 import Template
import sys
import re
print("jinja started..")
if len(sys.argv) != 3:
    print("Usage:\n python " + sys.argv[0] + " <input-template> <output-file>")
    sys.exit()

print("input: " + sys.argv[1])
print("output: " + sys.argv[2])

# open and read file
with open(sys.argv[1], 'r') as myfile:
    myfile_string=myfile.read() 

    # replace verilog define macros
    for match_define in re.finditer(r'`define .+ [0-9]+', myfile_string):
        define_string = match_define.group(0)
        define_name = define_string.split()[1]
        define_value = define_string.split()[2]
        myfile_string = myfile_string.replace("`" + define_name, define_value)

    # apply template
    template = Template(myfile_string)

    # write parsed file
    with open(sys.argv[2], "wb") as fh:
        fh.write(template.render())
