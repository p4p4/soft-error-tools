import sys

for filename in ["pipe_add", "pipe_add_tmr"]:
    for inputs in [1,2,8, 16, 32, 64, 128]:
        for layers in [1,2, 4, 6, 8, 10]:
            with open(filename + ".v", 'r') as myfile:
                file_as_str = myfile.read()
                file_as_str = file_as_str.replace("WORD_WIDTH 8", "WORD_WIDTH " + str(inputs))
                file_as_str = file_as_str.replace("LAYERS 2", "LAYERS " + str(layers))
                output_name = filename+ "_i" + str(inputs) + "_l" + str(layers)
                with open(output_name + ".v", "wb") as fh:
                    fh.write(file_as_str)
                    print output_name
