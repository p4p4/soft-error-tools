// this file uses jinja templates for code generation, see http://jinja.pocoo.org
`define WORD_WIDTH 4  // the bit width of the input, adders, and output signals
`define LAYERS 1      // defines how often the adder is cascaded

module top(
    input_vector,   // the input vector, WORD_WIDTH broad
    sum,            // the final result, WORD_WIDTH broad
    clk,
    alarm_signals
    );
    
    input   clk;
    input   [4-1:0] input_vector;
    output  [4-1:0] sum;

    wire [4-1:0] b_out_0 = input_vector;
    wire [4-1:0] result_0 = input_vector;

    output [1-1:0] alarm_signals;

    // -- begin unrolled code --

    wire [4-1:0] b_out_1;
    wire [4-1:0] result_1; 
    full_adder_nbit adder2(clk, result_0, b_out_0, result_1, b_out_1, alarm_signals[0]);

    // -- end unrolled code --


    assign sum = result_1;

endmodule // top module


module full_adder_nbit(
    clk,
    in_a,
    in_b,
    sum,
    out_b,
    alarm
    );
    
    input   [4-1:0] in_a;
    input   [4-1:0] in_b;
    input clk;
    output [4-1:0] sum;
    output [4-1:0] out_b;
    output alarm;
    
    wire [4-1:0] res;
    parity_protected_memory mem(clk, res, sum); 
    assign res = in_a + in_b;

    parity_protected_memory mem(clk, in_b, out_b, alarm);
endmodule // full_adder_nbit

module parity_protected_memory(clk, in,out, alarm);
    input clk;
    input  [4-1:0] in;
    output [4-1:0] out;
    output alarm;

    reg [4-1:0] memory;
    reg parity_reg;

    wire parity_in;
    wire parity_out;

    assign parity_in = in[0] ^ in[1] ^ in[2] ^ in[3];
    assign parity_out = out[0] ^ out[1] ^ out[2] ^ out[3];

    assign alarm = ~parity_reg ^ parity_out;
    assign out = memory;

    initial begin
        memory = 1'b0;
        parity_reg = 0;
    end

    always @(posedge clk) begin
        memory = in;
        parity_reg = parity_in;
    end
endmodule // parity_protected_memory


/*
module my_dff (q, clk, d);

output q;
input d;
input clk;
  reg reg_q;
  assign q = reg_q; // Blocking style
  always @(posedge clk) begin
    reg_q <= d;
  end
endmodule
*/
