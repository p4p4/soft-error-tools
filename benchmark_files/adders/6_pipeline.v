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
    input   [`WORD_WIDTH-1:0] input_vector;
    output  [`WORD_WIDTH-1:0] sum;

    wire [`WORD_WIDTH-1:0] b_out_0 = input_vector;
    wire [`WORD_WIDTH-1:0] result_0 = input_vector;

    output [`LAYERS-1:0] alarm_signals;

    // -- begin unrolled code --
{% for idx in range(1,`LAYERS+1) %}
    wire [`WORD_WIDTH-1:0] b_out_{{ idx }};
    wire [`WORD_WIDTH-1:0] result_{{ idx }}; 
    full_adder_nbit adder2(clk, result_{{idx - 1}}, b_out_{{idx - 1}}, result_{{idx}}, b_out_{{idx}}, alarm_signals[{{idx - 1}}]);
{% endfor %}
    // -- end unrolled code --


    assign sum = result_`LAYERS;

endmodule // top module


module full_adder_nbit(
    clk,
    in_a,
    in_b,
    sum,
    out_b,
    alarm
    );
    
    input   [`WORD_WIDTH-1:0] in_a;
    input   [`WORD_WIDTH-1:0] in_b;
    input clk;
    output [`WORD_WIDTH-1:0] sum;
    output [`WORD_WIDTH-1:0] out_b;
    output alarm;
    
    wire [`WORD_WIDTH-1:0] res;
    parity_protected_memory mem(clk, res, sum); 
    assign res = in_a + in_b;

    parity_protected_memory mem(clk, in_b, out_b, alarm);
endmodule // full_adder_nbit

module parity_protected_memory(clk, in,out, alarm);
    input clk;
    input  [`WORD_WIDTH-1:0] in;
    output [`WORD_WIDTH-1:0] out;
    output alarm;

    reg [`WORD_WIDTH-1:0] memory;
    reg parity_reg;

    wire parity_in;
    wire parity_out;

    assign parity_in = in[0]{% for idx in range(1,`WORD_WIDTH) %} ^ in[{{idx}}]{% endfor %};
    assign parity_out = out[0]{% for idx in range(1,`WORD_WIDTH) %} ^ out[{{idx}}]{% endfor %};

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

