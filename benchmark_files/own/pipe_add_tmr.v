// this file uses jinja templates for code generation, see http://jinja.pocoo.org
`define WORD_WIDTH 8  // the bit width of the input, adders, and output signals
`define LAYERS 2      // defines how often the adder is cascaded

module top(
    input_vector,   // the input vector, WORD_WIDTH broad
    sum,            // the final result, WORD_WIDTH broad
    clk,
    Err_out_Final   // the alarm output
    );
    
    input   clk;
    input   [`WORD_WIDTH-1:0] input_vector;
    output  [`WORD_WIDTH-1:0] sum;
    output  Err_out_Final;

    wire [`WORD_WIDTH-1:0] b_out_R1_0 = input_vector;
    wire [`WORD_WIDTH-1:0] result_R1_0 = input_vector;

    wire [`WORD_WIDTH-1:0] b_out_R2_0 = input_vector;
    wire [`WORD_WIDTH-1:0] result_R2_0 = input_vector;

    wire [`WORD_WIDTH-1:0] b_out_R3_0 = input_vector;
    wire [`WORD_WIDTH-1:0] result_R3_0 = input_vector;

    // -- begin unrolled code --
{% for idx in range(1,`LAYERS+1) %}
    wire [`WORD_WIDTH-1:0] b_out_R1_{{ idx }};
    wire [`WORD_WIDTH-1:0] result_R1_{{ idx }};
    full_adder_nbit adder_R1_{{idx}}(clk, result_R1_{{idx - 1}}, b_out_R1_{{idx - 1}}, result_R1_{{idx}}, b_out_R1_{{idx}});

    wire [`WORD_WIDTH-1:0] b_out_R2_{{ idx }};
    wire [`WORD_WIDTH-1:0] result_R2_{{ idx }};
    full_adder_nbit adder_R2_{{idx}}(clk, result_R2_{{idx - 1}}, b_out_R2_{{idx - 1}}, result_R2_{{idx}}, b_out_R2_{{idx}});

    wire [`WORD_WIDTH-1:0] b_out_R3_{{ idx }};
    wire [`WORD_WIDTH-1:0] result_R3_{{ idx }};
    full_adder_nbit adder_R3_{{idx}}(clk, result_R3_{{idx - 1}}, b_out_R3_{{idx - 1}}, result_R3_{{idx}}, b_out_R3_{{idx}});
{% endfor %}
    // -- end unrolled code --

    tmr tmr_module(result_R1_`LAYERS, result_R2_`LAYERS, result_R3_`LAYERS, sum);

    //assign Err_out_Final = 0;


endmodule // top module


module full_adder_nbit(
    clk,
    in_a,
    in_b,
    sum,
    out_b
    );
    
    input   [`WORD_WIDTH-1:0] in_a;
    input   [`WORD_WIDTH-1:0] in_b;
    input clk;
    output [`WORD_WIDTH-1:0] sum;
    output [`WORD_WIDTH-1:0] out_b;
    
    wire [`WORD_WIDTH-1:0] res; 
    assign res = in_a + in_b;

    d_ff result(clk, res, sum);
    d_ff input_delay(clk, in_b, out_b);

endmodule // full_adder_nbit

module d_ff(clk, in, out);
    input clk;
    input  [`WORD_WIDTH-1:0] in;
    output [`WORD_WIDTH-1:0] out;

    reg [`WORD_WIDTH-1:0] memory;

    assign out = memory;

    initial begin
        memory = 1'b0;
    end

    always @(posedge clk) begin
        memory = in;
    end
endmodule // d_ff

module tmr(in_1, in_2, in_3, out);
    input  [`WORD_WIDTH-1:0] in_1;
    input  [`WORD_WIDTH-1:0] in_2;
    input  [`WORD_WIDTH-1:0] in_3;
    output [`WORD_WIDTH-1:0] out;

    assign out = (in_1 & in_2) | (in_2 & in_3) | (in_1 & in_3);
    
endmodule


