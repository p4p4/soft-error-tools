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
    full_adder_nbit adder2(clk, result_{{idx - 1}}, b_out_{{idx - 1}}, result_{{idx}}, b_out_{{idx}}, hold_signals[{{idx - 1}}], error_signals[{{idx - 1}}]);
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
    hold,
    not_valid
    );
    
    input   [`WORD_WIDTH-1:0] in_a;
    input   [`WORD_WIDTH-1:0] in_b;
    input clk;
    input hold;
    output [`WORD_WIDTH-1:0] sum;
    output [`WORD_WIDTH-1:0] out_b;
    output not_valid;
    
    wire [`WORD_WIDTH-1:0] res; 
    assign res = in_a + in_b;



    // parity_protected_memory result(clk, res, sum, hold, sum_latches_invalid);
    reg [`WORD_WIDTH-1:0] memory;
    reg parity_reg;
    wire parity_in;
    wire parity_out;
    assign parity_in = res[0]{% for idx in range(1,`WORD_WIDTH) %} ^ res[{{idx}}]{% endfor %};
    assign parity_out = sum[0]{% for idx in range(1,`WORD_WIDTH) %} ^ sum[{{idx}}]{% endfor %};

    assign sum = memory;

    initial begin
        memory = 1'b0;
        parity_reg = 0;
    end

    always @(posedge clk) begin
        if (~hold) begin
            memory = res;
            parity_reg = parity_in;
        end
    end
    // END parity_protected_memory

    //parity_protected_memory input_delay(clk, in_b, out_b, hold, in_latches_invalid);
    reg [`WORD_WIDTH-1:0] memory_delay_;
    reg parity_reg_delay_;

    wire parity_in_d_;
    wire parity_out_d_;

    assign parity_in_d_ = in_b[0]{% for idx in range(1,`WORD_WIDTH) %} ^ in_b[{{idx}}]{% endfor %};
    assign parity_out_d_ = out_b[0]{% for idx in range(1,`WORD_WIDTH) %} ^ out_b[{{idx}}]{% endfor %};

    assign out_b = memory_delay_;

    initial begin
        memory_delay_ = 1'b0;
        parity_reg_delay_ = 0;
    end

    always @(posedge clk) begin
        if (~hold) begin
            memory_delay_ = in_b;
            parity_reg_delay_ = parity_in_d_;
        end
    end

    assign not_valid = (parity_reg_delay_ ^ parity_out_d_) | (parity_reg ^ parity_out);
endmodule // full_adder_nbit




module checker(hold_signals, error_signals, alarm_output);
    input [`LAYERS-1:0] hold_signals;
    input [`LAYERS-1:0] error_signals;
    output alarm_output;


    // -- begin unrolled code --
{% for i1 in range(0,`LAYERS) %}
    // error in layer {{i1}}:
    wire err_in_L{{i1}} = error_signals[{{i1}}]{% if i1 is not equalto `LAYERS - 1%}  & ~hold_signals[{{i1+1}}] {% endif %};
{% endfor %}

    assign alarm_output = err_in_L0{% for i1 in range(1,`LAYERS) %} | err_in_L{{i1}}{% endfor %};
    // -- end unrolled code --

endmodule // checker


