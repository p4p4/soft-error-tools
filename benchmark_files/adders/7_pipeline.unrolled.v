// this file uses jinja templates for code generation, see http://jinja.pocoo.org
`define WORD_WIDTH 1  // the bit width of the input, adders, and output signals
`define LAYERS 1      // defines how often the adder is cascaded

module top(
    input_vector,   // the input vector, WORD_WIDTH broad
    hold_signals,
    sum,            // the final result, WORD_WIDTH broad
    clk,
    Err_out_Final   // the alarm output
    );
    
    input   clk;
    input   [1-1:0] input_vector;
    input   [1-1:0] hold_signals;
    output  [1-1:0] sum;
    output  Err_out_Final;

    wire [1-1:0] b_out_0 = input_vector;
    wire [1-1:0] result_0 = input_vector;
    wire [1-1:0] error_signals;

    // -- begin unrolled code --

    wire [1-1:0] b_out_1;
    wire [1-1:0] result_1;
    full_adder_nbit adder2(clk, result_0, b_out_0, result_1, b_out_1, hold_signals[0], error_signals[0]);

    // -- end unrolled code --

    checker myChecker(hold_signals, error_signals, Err_out_Final);


    assign sum = result_1;

endmodule // top module


module full_adder_nbit(
    clk,
    in_a,
    in_b,
    sum,
    out_b,
    hold,
    not_valid
    );
    
    input   [1-1:0] in_a;
    input   [1-1:0] in_b;
    input clk;
    input hold;
    output [1-1:0] sum;
    output [1-1:0] out_b;
    output not_valid;
    
    wire [1-1:0] res; 
    assign res = in_a + in_b;

    wire sum_latches_invalid;
    wire in_latches_invalid;
    parity_protected_memory result(clk, res, sum, hold, sum_latches_invalid);
    parity_protected_memory input_delay(clk, in_b, out_b, hold, in_latches_invalid);

    assign not_valid = in_latches_invalid | sum_latches_invalid;
endmodule // full_adder_nbit

module parity_protected_memory(clk, in, out, hold, not_valid);
    input clk;
    input  [1-1:0] in;
    input hold;
    output [1-1:0] out;
    output not_valid;

    reg [1-1:0] memory;
    reg parity_reg;

    wire parity_in;
    wire parity_out;

    assign parity_in = in[0];
    assign parity_out = out[0];

    assign not_valid = parity_reg ^ parity_out;
    assign out = memory;

    initial begin
        memory = 1'b0;
        parity_reg = 0;
    end

    always @(posedge clk) begin
        if (~hold) begin
            memory = in;
            parity_reg = parity_in;
        end
    end
endmodule // parity_protected_memory


module checker(hold_signals, error_signals, alarm_output);
    input [1-1:0] hold_signals;
    input [1-1:0] error_signals;
    output alarm_output;


    // -- begin unrolled code --

    // error in layer 0:
    wire err_in_L0 = error_signals[0];


    assign alarm_output = err_in_L0;
    // -- end unrolled code --

endmodule // checker

