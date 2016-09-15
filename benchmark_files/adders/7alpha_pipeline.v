// this file uses jinja templates for code generation, see http://jinja.pocoo.org
`define WORD_WIDTH 4  // the bit width of the input, adders, and output signals
`define LAYERS 3      // defines how often the adder is cascaded

module top(
    input_vector,   // the input vector, WORD_WIDTH broad
    hold_signals,
    sum,            // the final result, WORD_WIDTH broad
    clk,
    Err_out_Final   // the alarm output
    );
    
    input   clk;
    input   [`WORD_WIDTH-1:0] input_vector;
    input   [`LAYERS-1:0] hold_signals;
    output  [`WORD_WIDTH-1:0] sum;
    output  Err_out_Final;

    wire [`WORD_WIDTH-1:0] b_out_0 = input_vector;
    wire [`WORD_WIDTH-1:0] result_0 = input_vector;
    wire [`LAYERS-1:0] error_signals;

    // -- begin unrolled code --
{% for idx in range(1,`LAYERS+1) %}
    wire [`WORD_WIDTH-1:0] b_out_{{ idx }};
    wire [`WORD_WIDTH-1:0] result_{{ idx }}; 
    full_adder_nbit adder2(clk, result_{{idx - 1}}, b_out_{{idx - 1}}, result_{{idx}}, b_out_{{idx}}, error_signals[{{idx - 1}}]);
{% endfor %}
    // -- end unrolled code --

    checker myChecker(hold_signals, error_signals, Err_out_Final);


    assign sum = result_`LAYERS;

endmodule // top module


module full_adder_nbit(
    clk,
    in_a,
    in_b,
    sum,
    out_b,
    valid
    );
    
    input   [`WORD_WIDTH-1:0] in_a;
    input   [`WORD_WIDTH-1:0] in_b;
    input clk;
    output [`WORD_WIDTH-1:0] sum;
    output [`WORD_WIDTH-1:0] out_b;
    output valid;
    
    wire [`WORD_WIDTH-1:0] res;
    parity_protected_memory mem(clk, res, sum); 
    assign res = in_a + in_b;

    parity_protected_memory mem(clk, in_b, out_b, valid);
endmodule // full_adder_nbit

module parity_protected_memory(clk, in,out, valid);
    input clk;
    input  [`WORD_WIDTH-1:0] in;
    output [`WORD_WIDTH-1:0] out;
    output valid;

    reg [`WORD_WIDTH-1:0] memory;
    reg parity_reg;

    wire parity_in;
    wire parity_out;

    assign parity_in = in[0]{% for idx in range(1,`WORD_WIDTH) %} ^ in[{{idx}}]{% endfor %};
    assign parity_out = out[0]{% for idx in range(1,`WORD_WIDTH) %} ^ out[{{idx}}]{% endfor %};

    assign valid = ~parity_reg ^ parity_out;
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


module checker(hold_signals, error_signals, alarm_output);
    input [`LAYERS-1:0] hold_signals;
    input [`LAYERS-1:0] error_signals;
    output alarm_output;


    // -- begin unrolled code --
{% for i1 in range(0,`LAYERS) %}
    // error in layer {{i1}}:
    wire err_in_L{{i1}} = error_signals[{{i1}}] {% for i2 in range(i1 +1,`LAYERS) %} & ~hold_signals[{{i2}}]{% endfor %};
{% endfor %}

    assign alarm_output = err_in_L0{% for i1 in range(1,`LAYERS) %} | err_in_L{{i1}}{% endfor %};
    // -- end unrolled code --

endmodule // checker



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



