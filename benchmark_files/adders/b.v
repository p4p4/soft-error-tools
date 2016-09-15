module parity_protected_memory(clk, in,out);
    parameter WORD_WIDTH = 4;

    reg [WORD_WIDTH-1:0] memory;
    //initial memory = 0;
    always @(posedge clk) memory = in;
    input  [WORD_WIDTH-1:0] in;
    input clk;
    output [WORD_WIDTH-1:0] out;

    assign out = memory;

endmodule
